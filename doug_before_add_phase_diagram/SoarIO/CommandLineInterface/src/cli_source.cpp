/////////////////////////////////////////////////////////////////
// source command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <fstream>

#include "cli_Constants.h"
#include "sml_StringOps.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSource(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	if (argv.size() < 2) {
		SetErrorDetail("Please supply one file to source. If there are spaces in the path, please enclose in quotes.");
		return SetError(CLIError::kTooFewArgs);

	} else if (argv.size() > 2) {
		// but only one filename
		return SetError(CLIError::kSourceOnlyOneFile);
	}

	return DoSource(pAgent, argv[1]);
}

bool CommandLineInterface::Trim(std::string& line) {
	// trim whitespace and comments from line
	if (!line.size()) return true; // nothing on the line

	// remove leading whitespace
	std::string::size_type pos = line.find_first_not_of(" \t");
	if (pos != std::string::npos) line = line.substr(pos);

	bool pipe = false;
	std::string::size_type searchpos = 0;

	for (pos = line.find_first_of("\\#|", searchpos); pos != std::string::npos; pos = line.find_first_of("\\#|", searchpos)) {
		switch (line[pos]) {
			case '\\':
				searchpos = pos + 2;
				break;

			case '#':
				if (pipe) {
					searchpos = pos + 1;
				} else {
					line = line.substr(0, pos);
				}
				break;

			case '|':
				pipe = !pipe;
				searchpos = pos + 1;
				break;
		}
	}
	if (pipe) return SetError(CLIError::kNewlineBeforePipe);
	return true;
}

bool CommandLineInterface::DoSource(gSKI::IAgent* pAgent, std::string filename) {
	if (!RequireAgent(pAgent)) return false;

    StripQuotes(filename);

    // Separate the path out of the filename if any
	std::string path;
	unsigned int separator1 = filename.rfind('/');
	if (separator1 != std::string::npos) {
		++separator1;
		if (separator1 < filename.length()) {
			path = filename.substr(0, separator1);
			filename = filename.substr(separator1, filename.length() - separator1);
			if (!DoPushD(path)) return false;
		}
	}
	unsigned int separator2 = filename.rfind('\\');
	if (separator2 != std::string::npos) {
		++separator2;
		if (separator2 < filename.length()) {
			path = filename.substr(0, separator2);
			filename = filename.substr(separator2, filename.length() - separator2);
			if (!DoPushD(path)) return false;
		}
	}

	// Open the file
	std::ifstream soarFile(filename.c_str());
	if (!soarFile) {
		if (path.length()) DoPopD();
		return SetError(CLIError::kOpenFileFail);
	}

	std::string line;				// Each line removed from the file
	std::string command;			// The command, sometimes spanning multiple lines
	std::string::size_type pos;		// Used to find braces on a line (triggering multiple line spanning commands)
	int braces = 0;					// Brace nest level (hopefully all braces are supposed to be closed)
	std::string::iterator iter;		// Iterator when parsing for braces and pounds
	int lineCount = 0;				// Count the lines per file
	int lineCountCache = 0;			// Used to save a line number

	// Set directory depth to zero on first call to source, even though it should be zero anyway
	if (m_SourceDepth == 0) {
		m_SourceDirDepth = 0;
	}
	++m_SourceDepth;

	// Go through each line of the file (Yay! C++ file parsing!)
	while (getline(soarFile, line)) {
	
		// Increment line count
		++lineCount;

		// Clear out the old command
		command.clear();

		// Trim whitespace and comments
		if (!Trim(line)) {
			HandleSourceError(lineCount, filename);
			if (path.length()) DoPopD();
			return false;
		}

		if (!line.length()) continue; // Nothing on line, skip it

		// If there is a brace on the line, concatenate lines until the closing brace
		pos = line.find('{');

		if (pos != std::string::npos) {
			
			// Save this line number for error messages
			lineCountCache = lineCount;

			// While we are inside braces, stay in special parsing mode
			do {
				if (lineCountCache != lineCount) {
					if (!Trim(line)) { // Trim whitespace and comments on additional lines
						HandleSourceError(lineCount, filename);
						if (path.length()) DoPopD();
						return false; 
					}
				}

				// nothing on line or just whitespace and comments
				if (!line.size()) continue;

				// Enter special parsing mode
				iter = line.begin();
				while (iter != line.end()) {
					// Go through each of the characters, counting brace nesting level
					if (*iter == '{') ++braces;
					else if (*iter == '}') --braces;

					// Next character
					++iter;
				}
				
				// We finished that line, add it to the command
				command += line;

				// Did we close all of the braces?
				if (!braces) break; // Yes, break out of special parsing mode

				// Did we go negative?
				if (braces < 0) break; // Yes, break out on error

				// Put the newline back on it (getline eats the newline)
				command += '\n';

				// We're getting another line, increment count now
				++lineCount;

				// Get the next line from the file and repeat
			} while (getline(soarFile, line));

			// Did we break out because of closed braces or EOF?
			if (braces > 0) {
				// EOF while still nested
				SetError(CLIError::kUnmatchedBrace);
				HandleSourceError(lineCountCache, filename);
				if (path.length()) DoPopD();
				return false;

			} else if (braces < 0) {
				SetError(CLIError::kExtraClosingBrace);
				HandleSourceError(lineCountCache, filename);
				if (path.length()) DoPopD();
				return false;
			}

			// We're good to go

		} else {
			// No braces on line, set command to line
			command = line;

			// Set cache to same line for error message
			lineCountCache = lineCount;
		}

		// Fire off the command
		unsigned oldResultSize = m_Result.str().size();
		if (DoCommandInternal(pAgent, command)) {
			// Add trailing newline if result changed size
			unsigned newResultSize = m_Result.str().size();
			if (oldResultSize != newResultSize) {
				// but don't add after sp's
				if (m_Result.str()[m_Result.str().size()-1] != '*') {
					m_Result << '\n';
				}
			}

		} else {
			// Command failed, error in result
			HandleSourceError(lineCountCache, filename);
			if (path.length()) DoPopD();
			return false;
		}	
	}

	// Completion
	--m_SourceDepth;

	// if we're returning to the user
	if (!m_SourceDepth) {
		// Print working directory if source directory depth !=  0
		if (m_SourceDirDepth != 0) DoPWD();	// Ignore error
		m_SourceDirDepth = 0;

		// Add finished message
		if (m_Result.str()[m_Result.str().size()-1] != '\n') m_Result << '\n';
		m_Result << "Source finished.";
	}

	soarFile.close();
	if (path.length()) DoPopD();
	return true;
}

void CommandLineInterface::HandleSourceError(int errorLine, const std::string& filename) {
	if (!m_SourceError) {

		// Output error message
		m_SourceErrorDetail.clear();
		m_SourceErrorDetail += "\nSource command error on (or near) line ";

		char buf[kMinBufferSize];
		m_SourceErrorDetail += Int2String(errorLine, buf, kMinBufferSize);

		m_SourceErrorDetail += " of ";
		
		std::string directory;
		GetCurrentWorkingDirectory(directory); // Again, ignore error here

		m_SourceErrorDetail += filename + " (" + directory + ")";

		// PopD to original source directory
		while (m_SourceDirDepth) {
			if (m_SourceDirDepth < 0) m_SourceDirDepth = 0; // don't loop forever
			DoPopD(); // Ignore error here since it will be rare and a message confusing
		}

		// Reset depth to zero
		m_SourceDepth = 0;

		m_SourceError = true;

	} else {
		char buf[kMinBufferSize];
		m_SourceErrorDetail += "\n\t--> Sourced by: " + filename + " (line " + Int2String(errorLine, buf, kMinBufferSize) + ")";
	}
}

