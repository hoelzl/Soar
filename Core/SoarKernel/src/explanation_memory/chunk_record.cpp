#include "chunk_record.h"

#include "action_record.h"
#include "condition_record.h"
#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "ebc.h"
#include "explanation_memory.h"
#include "identity_record.h"
#include "instantiation.h"
#include "instantiation_record.h"
#include "output_manager.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"
#include "visualize.h"

void chunk_record::init(agent* myAgent, uint64_t pChunkID)
{
    thisAgent                   = myAgent;
    name                        = NULL;
    type                        = ebc_no_rule;
    chunkID                     = pChunkID;
    chunkInstantiation          = NULL;
    chunkInstantiationID        = 0;
    original_productionID       = 0;
    excised_production          = NULL;
    time_formed                 = 0;
    match_level                 = 0;

    conditions                  = new condition_record_list;
    actions                     = new action_record_list;

    baseInstantiation           = NULL;
    result_instantiations       = new inst_set();
    result_inst_records         = new inst_record_set();

    backtraced_instantiations   = new inst_set();
    backtraced_inst_records     = new inst_record_list();

    identity_analysis.init(thisAgent);

    stats.duplicates                        = 0;
    stats.tested_local_negation             = false;
    stats.tested_deep_copy                  = false;
    stats.tested_quiescence                 = false;
    stats.tested_ltm_recall                 = false;
    stats.reverted                          = false;
    stats.lhs_unconnected                   = false;
    stats.rhs_unconnected                   = false;
    stats.repaired                          = false;
    stats.repair_failed                     = false;
    stats.did_not_match_wm                  = false;
    stats.grounding_conditions_added    = 0;
    stats.merged_conditions                 = 0;
    stats.merged_disjunctions               = 0;
    stats.merged_disjunction_values         = 0;
    stats.eliminated_disjunction_values     = 0;
    stats.instantations_backtraced          = 0;
    stats.seen_instantations_backtraced     = 0;
    stats.constraints_attached              = 0;
    stats.constraints_collected             = 0;

    stats.identities_created                = 0;
    stats.identities_joined                 = 0;
    stats.identities_joined_variable        = 0;
    stats.identities_joined_local_singleton = 0;
    stats.identities_joined_singleton       = 0;
    stats.identities_joined_child_results   = 0;
    stats.identities_literalized_rhs_literal    = 0;
    stats.identities_participated           = 0;
    stats.identity_propagations             = 0;
    stats.identity_propagations_blocked     = 0;
    stats.operational_constraints           = 0;
    stats.OSK_instantiations                = 0;
    stats.identities_literalized_rhs_func_arg       = 0;
    stats.identities_literalized_rhs_func_compare   = 0;

    dprint(DT_EXPLAIN, "Created new empty chunk record c%u\n", chunkID);
}

void chunk_record::clean_up()
{
    dprint(DT_EXPLAIN, "Deleting chunk record %y (c %u)\n", name, chunkID);
//    production_remove_ref(thisAgent, original_production);
    production* originalProduction = thisAgent->explanationMemory->get_production(original_productionID);
    if (originalProduction)
    {
        originalProduction->save_for_justification_explanation = false;
    }
    if (name) thisAgent->symbolManager->symbol_remove_ref(&name);
    if (conditions) delete conditions;
    if (actions) delete actions;
    if (result_instantiations) delete result_instantiations;
    if (result_inst_records) delete result_inst_records;
    if (backtraced_inst_records) delete backtraced_inst_records;
    if (backtraced_instantiations) delete backtraced_instantiations;

    identity_analysis.clean_up();
    dprint(DT_EXPLAIN, "Done deleting chunk record c%u\n", chunkID);
}

void chunk_record::record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_join_map* pIdentitySetMappings, instantiation* pBaseInstantiation, tc_number pBacktraceNumber, instantiation* pChunkInstantiation)
{
    name = pProduction->name;
    thisAgent->symbolManager->symbol_add_ref(name);
    original_productionID = thisAgent->explanationMemory->add_production_id_if_necessary(pProduction);
    pProduction->save_for_justification_explanation = true;

    time_formed = thisAgent->d_cycle_count;
    match_level = pChunkInstantiation->match_goal_level;
    chunkInstantiationID = pChunkInstantiation->i_id;
    chunkInstantiation  = thisAgent->explanationMemory->add_instantiation(pChunkInstantiation, 0, true);

    conditions         = new condition_record_list;
    actions            = new action_record_list;

    instantiation_record* lResultInstRecord, *lNewInstRecord;
    instantiation* lNewInst;

    /* The following creates instantiations records for all instantiation involved in the backtrace.
     *
     * Note that instantiation records are shared between multiple chunk explanations, so
     * we only create a new instantiation record for the first chunk explanation that
     * backtraces through that instantiation.  If an instantiation already exists, we mark it for
     * an update, since the backtrace info will have changed for the new rule being explained. */

    dprint(DT_EXPLAIN, "(1) Recording all bt instantiations (%d instantiations)...\n", backtraced_instantiations->size());
    for (auto it = backtraced_instantiations->begin(); it != backtraced_instantiations->end(); it++)
    {
        lNewInst = (*it);
        lNewInstRecord = thisAgent->explanationMemory->add_instantiation((*it), chunkID);
        assert(lNewInstRecord);
        backtraced_inst_records->push_back(lNewInstRecord);
        dprint(DT_EXPLAIN, "%u (%y)\n", (*it)->i_id, (*it)->prod_name);
    }
    dprint(DT_EXPLAIN, "There are now %d instantiations records...\n", backtraced_inst_records->size());

    /* The following actually records the contents of the instantiation.
     *
     * Note that if step 1 detected that this is an instantiation that it already recorded for a
     * previous chunk, we call an update function . */

    for (auto it = backtraced_inst_records->begin(); it != backtraced_inst_records->end(); it++)
    {
        lNewInstRecord = (*it);
        dprint(DT_EXPLAIN, "Recording instantiation i%u.\n", lNewInstRecord->get_instantiationID());
        if (lNewInstRecord->cached_inst->explain_status == explain_recording)
        {
            lNewInstRecord->record_instantiation_contents();

        } else if (lNewInstRecord->cached_inst->explain_status == explain_recording_update)
        {
            lNewInstRecord->update_instantiation_contents();
        }
        lNewInstRecord->cached_inst->explain_status = explain_recorded;
    }

    baseInstantiation = thisAgent->explanationMemory->get_instantiation(pBaseInstantiation);

    dprint(DT_EXPLAIN, "(2) Recording other result instantiation of chunk...\n", pBaseInstantiation->i_id);
    for (auto it = result_instantiations->begin(); it != result_instantiations->end(); ++it)
    {
        lResultInstRecord = thisAgent->explanationMemory->get_instantiation((*it));
        assert(lResultInstRecord);
        result_inst_records->insert(lResultInstRecord);
    }

    dprint(DT_EXPLAIN, "(4) Recording conditions of chunk...\n");

    if (pChunkInstantiation->explain_status == explain_recorded)
    {
        pChunkInstantiation->explain_status = explain_recording_update;
        chunkInstantiation->update_instantiation_contents(true);
    } else {
        chunkInstantiation->record_instantiation_contents(true);
    }
    chunkInstantiation->cached_inst->explain_status = explain_recorded;

    /* Create condition and action records */
    instantiation_record* lchunkInstRecord;
    instantiation* lChunkCondInst = NULL;
    condition_record* lcondRecord;

    for (condition* cond = lhs; cond != NIL; cond = cond->next)
    {
        lChunkCondInst = cond->inst;
        if ((type == ebc_no_rule) && (cond->type == POSITIVE_CONDITION) )
        {
            if (cond->data.tests.id_test->eq_test->data.referent->is_sti())
            {
                type = ebc_justification;
            } else {
                type = ebc_chunk;
            }
        }
        if (lChunkCondInst)
        {
            dprint(DT_EXPLAIN, "Matching chunk condition %l from instantiation i%u (%y)", cond, lChunkCondInst->i_id, lChunkCondInst->prod_name);
            /* The backtrace should have already added all instantiations that contained
             * grounds, so we can just look up the instantiation for each condition */
            lchunkInstRecord = thisAgent->explanationMemory->get_instantiation(lChunkCondInst);
            if (!lchunkInstRecord)
            {
                /* I think this can only occur if a condition was testing a wme created as a child of a previous result, so the instantiation
                 * that originally created it is not in the backtrace 
                 * 
                 * Bug:  If a second explanation also has a condition that tests a wme created by this rule, problems will occur
                 *       It won't be detected here, since it was added for the first explanation, so it won't be added it to its 
                 *       backtraced instantiations and its path to a result won't be updated.  We need a better mechanism, but we'll 
                 *       punt for now since this only happens for explanations with partially operational conditions that are repaired. 
                 *       FYI, no longer crashes.  But the rule won't show up properly in the visualization.  */
                dprint(DT_EXPLAIN, "Adding missing instantiation record for i%u (%y)", lChunkCondInst->i_id, lChunkCondInst->prod_name);
                lchunkInstRecord = thisAgent->explanationMemory->add_instantiation(lChunkCondInst, chunkID);
                lchunkInstRecord->record_instantiation_contents();
                /* MToDo | Don't think we need this.  We know the instantiation was never recorded and update is only for shared instantiations */
                //lchunkInstRecord->update_instantiation_contents();
                lchunkInstRecord->cached_inst->explain_status = explain_recorded;
                backtraced_inst_records->push_back(lNewInstRecord);
            }
        } else {
            lchunkInstRecord = NULL;
        }
        lcondRecord = thisAgent->explanationMemory->add_condition(conditions, cond, lchunkInstRecord);
        lcondRecord->set_instantiation(lchunkInstRecord);
    }
    dprint(DT_EXPLAIN, "...done with (4) adding chunk instantiation conditions!\n");

    dprint(DT_EXPLAIN, "(5) Recording actions...\n");

    action_record* new_action_record;
    preference* pref;
    action* lAction;
    for (pref = results, lAction= rhs; (pref != NIL) && (lAction != NIL); pref = pref->next_result, lAction = lAction->next)
    {
        new_action_record = thisAgent->explanationMemory->add_result(pref, lAction, false);
        actions->push_back(new_action_record);
    }

    dprint(DT_EXPLAIN, "(6) Recording identity mappings...\n");

    identity_analysis.analyze_chunk_identities(chunkInstantiationID, lhs);
    auto lGoalIter = thisAgent->explanationMemory->all_identities_in_goal->find(thisAgent->explanationBasedChunker->m_inst->match_goal);
    assert(lGoalIter != thisAgent->explanationMemory->all_identities_in_goal->end());
    identity_analysis.record_identity_sets(lGoalIter->second);
    dprint(DT_EXPLAIN, "DONE recording chunk contents...\n");
}


void chunk_record::generate_dependency_paths()
{
    dprint(DT_EXPLAIN_PATHS, "Creating paths for chunk %u...\n", chunkID);

    inst_record_list* lInstPath = new inst_record_list();

    /* MToDo | Need to add OSK instantiations */
    baseInstantiation->create_identity_paths(lInstPath);
    for (auto it = result_inst_records->begin(); it != result_inst_records->end(); it++)
    {
        (*it)->create_identity_paths(lInstPath);
    }
    chunkInstantiation->create_identity_paths(lInstPath);

    delete lInstPath;

    dprint(DT_EXPLAIN_PATHS, "Getting paths for chunk inst %u's conditions...\n", chunkID);
    condition_record* l_cond;
    instantiation_record* l_inst;
    inst_record_list* l_path;

    for (condition_record_list::iterator it = chunkInstantiation->conditions->begin(); it != chunkInstantiation->conditions->end(); it++)
    {
        l_cond = (*it);
        l_inst = l_cond->get_instantiation();
        if (l_inst)
        {
            l_path = l_inst->get_path_to_base();
            /* This is to handle the problem situation that can occur with partially operational conditions that
             * are repaired.  Described above in record_chunk_contents. */
            if (l_path)
            {
                dprint(DT_EXPLAIN_PATHS, "Path to base of length %d for chunk inst cond (%u: %t ^%t %t) found from instantiation i%u (%y): \n", l_path->size(), l_cond->conditionID, l_cond->condition_tests.id, l_cond->condition_tests.attr, l_cond->condition_tests.value, l_inst->get_instantiationID(), l_inst->production_name);
                l_cond->set_path_to_base(l_path);
            } else dprint(DT_EXPLAIN_PATHS, "No chunk inst path returned for chunk cond (%u: %t ^%t %t) in instantiation %u (%y)!\n", l_cond->conditionID, l_cond->condition_tests.id, l_cond->condition_tests.attr, l_cond->condition_tests.value, l_inst->instantiationID, l_inst->production_name);

        } else dprint(DT_EXPLAIN_PATHS, "Condition (%u: %t ^%t %t) in chunk_inst has no parent instantiation!\n", l_cond->conditionID, l_cond->condition_tests.id, l_cond->condition_tests.attr, l_cond->condition_tests.value);
    }
    dprint(DT_EXPLAIN_PATHS, "Getting paths for chunk record %u's conditions...\n", chunkID);
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        l_cond = (*it);
        l_inst = l_cond->get_instantiation();
        if (l_inst)
        {
            l_path = l_inst->get_path_to_base();
            /* This is to handle the problem situation that can occur with partially operational conditions that
             * are repaired.  Described above in record_chunk_contents. */
            if (l_path)
            {
                dprint(DT_EXPLAIN_PATHS, "Path to base of length %d for chunk cond (%u: %t ^%t %t) found from instantiation i%u (%y): \n", l_path->size(), l_cond->conditionID, l_cond->condition_tests.id, l_cond->condition_tests.attr, l_cond->condition_tests.value, l_inst->get_instantiationID(), l_inst->production_name);
                l_cond->set_path_to_base(l_path);
            } else dprint(DT_EXPLAIN_PATHS, "No path returned for chunk cond (%u: %t ^%t %t) in instantiation %u (%y)!\n", l_cond->conditionID, l_cond->condition_tests.id, l_cond->condition_tests.attr, l_cond->condition_tests.value, l_inst->instantiationID, l_inst->production_name);
        } else dprint(DT_EXPLAIN_PATHS, "Condition (%u: %t ^%t %t) has no parent instantiation!\n", l_cond->conditionID, l_cond->condition_tests.id, l_cond->condition_tests.attr, l_cond->condition_tests.value);
    }
}

void chunk_record::end_chunk_record()
{
    if (backtraced_instantiations)
    {
        backtraced_instantiations->clear();
        result_instantiations->clear();
    }
}

void chunk_record::print_for_explanation_trace()
{

    chunkInstantiation->print_for_explanation_trace(true, false);

    Output_Manager* outputManager = thisAgent->outputManager;

    outputManager->set_column_indent(0, 7);
    outputManager->set_column_indent(1, 60);
    outputManager->set_column_indent(2, 110);

    outputManager->printa_sf(thisAgent, "                  %-Using variable identity IDs   %-Shortest Path to Result Instantiation\n\n");
    outputManager->printa_sf(thisAgent, "sp {%y\n", name);

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;

        outputManager->set_print_test_format(true, false);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     }\n");
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     -{\n");
                    lInNegativeConditions = true;
                }
            }
            outputManager->printa_sf(thisAgent, "%d:%-", lConditionCount);

            outputManager->printa_sf(thisAgent, "(%t%s^%t %t%s)%s%-",
                lCond->condition_tests.id, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                lCond->condition_tests.attr, lCond->condition_tests.value, lCond->test_for_acceptable_preference ? " +" : "",
                thisAgent->explanationMemory->is_condition_related(lCond) ? "*" : "");
            outputManager->printa_sf(thisAgent, "(%g%s^%g %g%s)%-",
                lCond->condition_tests.id, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                lCond->condition_tests.attr, lCond->condition_tests.value, lCond->test_for_acceptable_preference ? " +" : "");

            thisAgent->explanationMemory->print_path_to_base(lCond->get_path_to_base(), true);
            outputManager->printa(thisAgent, "\n");
        }
        if (lInNegativeConditions)
        {
            outputManager->printa(thisAgent, "     }\n");
        }
    }
    outputManager->printa(thisAgent, "      -->\n");

    /* For chunks, actual rhs is same as explanation trace without identity information on the rhs*/
    thisAgent->explanationMemory->print_chunk_actions(actions, thisAgent->explanationMemory->get_production(original_productionID), excised_production);
    outputManager->printa(thisAgent, "}\n\n");
}

void chunk_record::print_for_wme_trace()
{
    Output_Manager* outputManager = thisAgent->outputManager;

    outputManager->set_column_indent(0, 7);
    outputManager->set_column_indent(1, 60);
    outputManager->set_column_indent(2, 110);
    outputManager->printa_sf(thisAgent, "                   %-Instantiation that created matched WME\n\n");
    outputManager->printa_sf(thisAgent, "sp {%y\n", name);

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;

        outputManager->set_print_test_format(true, false);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     }\n");
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     -{\n");
                    lInNegativeConditions = true;
                }
            }
            outputManager->printa_sf(thisAgent, "%d:%-", lConditionCount);

            if (lCond->matched_wme.id)
            {
                outputManager->printa_sf(thisAgent, "(%y ^%y %y%s)%s",
                    lCond->matched_wme.id, lCond->matched_wme.attr, lCond->matched_wme.value,
                    lCond->test_for_acceptable_preference ? " +" : "",
                    thisAgent->explanationMemory->is_condition_related(lCond) ? "*" : "");
            } else {
                outputManager->printa_sf(thisAgent, "(N/A)%s", thisAgent->explanationMemory->is_condition_related(lCond) ? "*" : "");
            }
            if (lCond->matched_wme.id)
            {
                instantiation_record* lInstRecord = lCond->get_instantiation();
                bool isSuper = (match_level > 0) && (lCond->wme_level_at_firing < match_level);
                if (lInstRecord)
                {
                    outputManager->printa_sf(thisAgent, "%-i %u (%y)\n",
                        lInstRecord->instantiationID,
                        lInstRecord->production_name);
                } else if (lCond->type == POSITIVE_CONDITION)
                {
                    outputManager->printa_sf(thisAgent, isSuper ? "%-Higher-level Problem Space\n" : "%-Soar Architecture\n");
                } else {
                    outputManager->printa_sf(thisAgent, "%-N/A\n");
                }
            } else {
                outputManager->printa(thisAgent, "\n");
            }
        }
        if (lInNegativeConditions)
        {
            outputManager->printa(thisAgent, "     }\n");
        }
    }
    outputManager->printa(thisAgent, "      -->\n");

    /* For chunks, actual rhs is same as explanation trace without identity information on the rhs*/
    thisAgent->explanationMemory->print_chunk_actions(actions, thisAgent->explanationMemory->get_production(original_productionID), excised_production);
    outputManager->printa(thisAgent, "}\n");
}

void chunk_record::visualize()
{
    if (thisAgent->visualizationManager->settings->rule_format->get_value() == viz_name)
    {
        chunkInstantiation->viz_simple_instantiation(viz_simple_inst);
    } else {
        if (thisAgent->explanationMemory->print_explanation_trace)
        {
            chunkInstantiation->viz_et_instantiation(viz_chunk_record);
        } else {
            chunkInstantiation->viz_wm_instantiation(viz_chunk_record);
        }
    }
//    chunkInstantiation->viz_connect_conditions(true);
    thisAgent->visualizationManager->viz_connect_inst_to_chunk(baseInstantiation->get_instantiationID(), chunkInstantiation->get_instantiationID());
    for (auto it = result_inst_records->begin(); it != result_inst_records->end(); ++it)
    {
        thisAgent->visualizationManager->viz_connect_inst_to_chunk((*it)->get_instantiationID(), chunkInstantiation->get_instantiationID());
    }
    return;

    Output_Manager* outputManager = thisAgent->outputManager;
    GraphViz_Visualizer* visualizer = thisAgent->visualizationManager;
    condition_record* lCond;

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
        assert(false);
    }
    else
    {
        bool lInNegativeConditions = false;
        int lConditionCount = 0;

        thisAgent->outputManager->set_print_test_format(false, true);
        visualizer->viz_object_start(name, chunkID, viz_chunk_record);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);

            ++lConditionCount;
            if (lConditionCount > 1)
                visualizer->viz_endl();

            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    visualizer->viz_NCC_end();
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    visualizer->viz_NCC_start();
                    lInNegativeConditions = true;
                }
            }
            lCond->visualize_for_chunk();
        }
        if (lInNegativeConditions)
        {
            visualizer->viz_NCC_end();
        } else {
            visualizer->viz_endl();
        }
        visualizer->viz_seperator();
        action_record::viz_action_list(thisAgent, actions, thisAgent->explanationMemory->get_production(original_productionID));
        visualizer->viz_object_end(viz_chunk_record);
    }

    visualizer->viz_connect_inst_to_chunk(baseInstantiation->get_instantiationID(), chunkID);
    for (auto it = result_inst_records->begin(); it != result_inst_records->end(); ++it)
    {
        visualizer->viz_connect_inst_to_chunk((*it)->get_instantiationID(), chunkID);
    }
}

/* This function is not currently used, but I'm leaving in case we ever add something
 * that needs to excise explanations, for example a command or something to limit
 * memory use.  This function has not been tested. */
void chunk_record::excise_chunk_record()
{
    for (auto it = conditions->begin(); it != conditions->end(); it++)
    {
        thisAgent->explanationMemory->delete_condition((*it)->get_conditionID());
    }
    for (auto it = actions->begin(); it != actions->end(); it++)
    {
        thisAgent->explanationMemory->delete_action((*it)->get_actionID());
    }
    /* For this to work, store id of last chunk that created instantiation record.  If it's the
     * same as this chunk being excised, no other chunk uses it.  This assumes that this is only
     * called when a chunk fails.  If we need to excise records in general, we'll need refcounts */
    for (auto it = backtraced_inst_records->begin(); it != backtraced_inst_records->end(); it++)
    {
        if ((*it)->get_chunk_creator() == chunkID)
        {
            (*it)->delete_instantiation();
            thisAgent->explanationMemory->delete_instantiation((*it)->get_instantiationID());
        }
    }
}
