#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "cli_GetOpt.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePWatch(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"disable",	0, 0, 'd'},
		{"enable",	0, 0, 'e'},
		{"off",		0, 0, 'd'},
		{"onn",		0, 0, 'e'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	bool setting = false;
	bool query = true;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "de", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'd':
				setting = false;
				query = false;
				break;
			case 'e':
				setting = true;
				query = false;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}
	if (argv.size() > static_cast<unsigned>(GetOpt::optind) + 1) return m_Error.SetError(CLIError::kTooManyArgs);

	if (argv.size() == static_cast<unsigned>(GetOpt::optind) + 1) {
		return DoPWatch(pAgent, query, &argv[GetOpt::optind], setting);
	}

	return DoPWatch(pAgent);
}

bool CommandLineInterface::DoPWatch(gSKI::IAgent* pAgent, bool query, std::string* pProduction, bool setting) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (query || !pProduction) return m_Error.SetError(CLIError::kOptionNotImplemented);

	if (setting) {
		if (!pKernelHack->BeginTracingProduction(pAgent, pProduction->c_str())) return m_Error.SetError(CLIError::kProductionNotFound);
	} else {
		if (!pKernelHack->StopTracingProduction(pAgent, pProduction->c_str())) return m_Error.SetError(CLIError::kProductionNotFound);
	}

	return true;
}

