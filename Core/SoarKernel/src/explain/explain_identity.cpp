#include "explain.h"
#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "ebc.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "symbol.h"
#include "test.h"
#include "output_manager.h"
#include "working_memory.h"



identity_record::identity_record(agent* myAgent, chunk_record* pChunkRecord, id_to_id_map_type* pIdentitySetMappings)
{
    thisAgent = myAgent;
    original_ebc_mappings = new id_to_id_map_type();
    id_to_id_set_mappings = new id_to_idset_map_type();
    (*original_ebc_mappings) = (*pIdentitySetMappings);
    identities_in_chunk = new id_set();
    id_set_counter = 99;

}

identity_record::~identity_record()
{
    delete original_ebc_mappings;
    for (auto it = id_to_id_set_mappings->begin(); it != id_to_id_set_mappings->end(); ++it)
    {
        if (it->second->rule_variable) symbol_remove_ref(thisAgent, it->second->rule_variable);
        delete it->second;
    }
    delete id_to_id_set_mappings;
    delete identities_in_chunk;
}

void identity_record::add_identities_in_test(test pTest)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
        for (cons* c = pTest->data.conjunct_list; c != NULL; c = c->rest)
        {
            add_identities_in_test(static_cast<test>(c->first));
        }
    } else if (test_has_referent(pTest)) {
        if (pTest->identity)
        {
            if (identities_in_chunk->find(pTest->identity) == identities_in_chunk->end())
            {
                identities_in_chunk->insert(pTest->identity);
//                dprint(DT_EXPLAIN_IDENTITIES, "Adding identity mapping %u -> %u", pTest->identity, id_set_counter);
                identity_set_info* lNewIDSet = new identity_set_info();
                lNewIDSet->identity_set_ID = ++id_set_counter;
                lNewIDSet->rule_variable = pTest->data.referent;
                symbol_add_ref(thisAgent, lNewIDSet->rule_variable);
                id_to_id_set_mappings->insert({pTest->identity, lNewIDSet});
            }
        }
    }
}

void identity_record::add_identities_in_condition_list(condition* lhs)
{
    for (condition* lCond = lhs; lCond != NULL; lCond = lCond->next)
    {
        if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            add_identities_in_condition_list(lCond->data.ncc.top);
        } else {
            thisAgent->outputManager->set_dprint_test_format(DT_EXPLAIN_IDENTITIES, true, true);
            add_identities_in_test(lCond->data.tests.id_test);
            add_identities_in_test(lCond->data.tests.attr_test);
            add_identities_in_test(lCond->data.tests.value_test);
        }
    }
}
//#include "output_manager.h"
void identity_record::generate_identity_sets(condition* lhs)
{
    dprint(DT_EXPLAIN_IDENTITIES, "Building identity mappings based on conditions of chunk...\n");
    condition_record* l_cond;
    instantiation_record* l_inst;
    inst_record_list* l_path;

    /* Generate identity sets and add mappings for all conditions in chunk */
    add_identities_in_condition_list(lhs);
    /* MToDo | Will need to do for all base instantiations */
    print_identities_in_chunk();
    print_identity_mappings();
    print_original_ebc_mappings();

    /* Add mappings for other instantiations's identities based on original ebc_mappings */
    std::unordered_map< uint64_t, uint64_t >::iterator iter;
    identity_set_info* lNewIDSet;
    for (iter = original_ebc_mappings->begin(); iter != original_ebc_mappings->end(); ++iter)
    {
        lNewIDSet = new identity_set_info();
        if (iter->second != NULL_IDENTITY_SET)
        {
            id_to_idset_map_iter_type lIter = id_to_id_set_mappings->find(iter->second);
            assert (lIter != id_to_id_set_mappings->end());
            lNewIDSet->identity_set_ID = lIter->second->identity_set_ID;
            lNewIDSet->rule_variable = id_to_id_set_mappings->find(iter->second)->second->rule_variable;
            symbol_add_ref(thisAgent, lNewIDSet->rule_variable);
        } else {
            lNewIDSet->identity_set_ID = NULL_IDENTITY_SET;
            lNewIDSet->rule_variable = NULL;
        }
        id_to_id_set_mappings->insert({iter->first, lNewIDSet});
    }

    print_identity_mappings();
}


void identity_record::print_identity_mappings_for_instantiation(instantiation_record* pInstRecord)
{

}

void identity_record::print_identities_in_chunk()
{
    dprint_noprefix(DT_EXPLAIN_IDENTITIES, "\nIdentities in chunk (%u): ", identities_in_chunk->size());
    for (auto it = identities_in_chunk->begin(); it != identities_in_chunk->end(); ++it)
    {
        dprint_noprefix(DT_EXPLAIN_IDENTITIES, "%u ", (*it));
    }
}
void identity_record::print_identity_mappings()
{
    thisAgent->outputManager->set_column_indent(0, 3);
    thisAgent->outputManager->set_column_indent(1, 10);
    thisAgent->outputManager->set_column_indent(2, 17);
    thisAgent->outputManager->set_column_indent(3, 24);

    dprint_noprefix(DT_EXPLAIN_IDENTITIES, "\n\nMappings to set (%u):\n", id_to_id_set_mappings->size());
    for (auto it = id_to_id_set_mappings->begin(); it != id_to_id_set_mappings->end(); ++it)
    {
        dprint_noprefix(DT_EXPLAIN_IDENTITIES, "%-%u %--> %-%u %-(%y)\n", it->first, it->second->identity_set_ID, it->second->rule_variable);
    }
}

void identity_record::print_original_ebc_mappings()
{
    thisAgent->outputManager->set_column_indent(0, 6);
    thisAgent->outputManager->set_column_indent(1, 26);
    thisAgent->outputManager->set_column_indent(2, 31);
    thisAgent->outputManager->printa_sf(thisAgent, "ID %-Original %-Set %-Final\n\n");

    dprint_noprefix(DT_EXPLAIN_IDENTITIES, "\nOriginal EBC Mappings (%u):\n", original_ebc_mappings->size());
    std::unordered_map< uint64_t, uint64_t >::iterator iter;
    for (iter = original_ebc_mappings->begin(); iter != original_ebc_mappings->end(); ++iter)
    {
        dprint_noprefix(DT_EXPLAIN_IDENTITIES, "%u%-%y %-%u%-%y\n",
            iter->first, thisAgent->ebChunker->get_ovar_for_o_id(iter->first), iter->second, thisAgent->ebChunker->get_ovar_for_o_id(iter->second));
    }
}

void identity_record::print_identity_explanation(chunk_record* pChunkRecord)
{
//    for (condition_record_list::iterator it = pChunkRecord->conditions->begin(); it != pChunkRecord->conditions->end(); it++)
//    {
//        l_cond = (*it);
//        dprint(DT_EXPLAIN, "Adding identities in chunk cond %l: \n", l_cond);
//    }
//
//    baseInstantiation->create_identity_paths(lInstPath);
//    for (auto it = result_inst_records->begin(); it != result_inst_records->end(); it++)
//    {
//        (*it)->create_identity_paths(lInstPath);
//    }
//
//    delete lInstPath;


}
