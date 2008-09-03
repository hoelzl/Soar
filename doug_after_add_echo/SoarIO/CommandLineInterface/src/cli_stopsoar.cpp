/////////////////////////////////////////////////////////////////
// stop-soar command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "IgSKI_Agent.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Kernel.h"
#include "sml_KernelSML.h"

using namespace cli;

bool CommandLineInterface::ParseStopSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"self",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	bool self = false;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "s", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 's':
				self = true;
				break;
			case '?':
				{
					std::string detail;
					if (m_pGetOpt->GetOptOpt()) {
						detail = static_cast<char>(m_pGetOpt->GetOptOpt());
					} else {
						detail = argv[m_pGetOpt->GetOptind() - 1];
					}
					SetErrorDetail("Bad option '" + detail + "'.");
				}
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Concatinate remaining args for 'reason'
	if (m_pGetOpt->GetAdditionalArgCount()) {
		std::string reasonForStopping;
		unsigned int optind = m_pGetOpt->GetOptind();
		while (optind < argv.size()) reasonForStopping += argv[optind++] + ' ';
		return DoStopSoar(pAgent, self, &reasonForStopping);
	}
	return DoStopSoar(pAgent, self);
}

bool CommandLineInterface::DoStopSoar(gSKI::IAgent* pAgent, bool self, const std::string* reasonForStopping) {

	unused(reasonForStopping);

	if (self) {
		if (!RequireAgent(pAgent)) return false;
		if (!pAgent->Interrupt(gSKI_STOP_AFTER_DECISION_CYCLE, gSKI_STOP_BY_RETURNING, &m_gSKIError)) {
			SetErrorDetail("Error interrupting agent.");
			return SetError(CLIError::kgSKIError);
		}
		if (gSKI::isError(m_gSKIError)) return SetError(CLIError::kgSKIError);
		return true;
	} else {
		if (!RequireKernel()) return false;

		// Make sure the system stop event will be fired at the end of the run.
		// We used to call FireSystemStop() in this function, but that's no good because
		// it comes before the agent has stopped because interrupt only stops at the next
		// phase or similar boundary (so could be a long time off).
		// So instead we set a flag and allow system stop to fire at the end of the run.
		m_pKernelSML->RequireSystemStop(true) ;

		if (!m_pKernel->GetAgentManager()->InterruptAll(gSKI_STOP_AFTER_DECISION_CYCLE, &m_gSKIError)) {
			SetErrorDetail("Error interrupting all agents.");
			return SetError(CLIError::kgSKIError);
		}
		if (gSKI::isError(m_gSKIError)) return SetError(CLIError::kgSKIError);
		return true;
	}
}

