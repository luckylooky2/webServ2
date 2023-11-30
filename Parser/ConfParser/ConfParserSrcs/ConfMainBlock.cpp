#include "../ConfParserHeaders/ConfMainBlock.hpp"
#include <cstdlib>

#include <iostream>

// CONF::httpBlock		CONF::mainBlock::m_Http_block;		     
CONF::mainBlock*	CONF::mainBlock::instance = NULL;

// public
CONF::mainBlock*	CONF::mainBlock::initInstance(const std::string& file) {
	if (!CONF::mainBlock::instance) {
		CONF::mainBlock::instance = new CONF::mainBlock(file);
	}
	return CONF::mainBlock::instance;
}

CONF::mainBlock*	CONF::mainBlock::getInstance() {
	return CONF::mainBlock::instance;
}

// private
CONF::mainBlock::mainBlock(const std::string& file)
 : m_Daemon(false),
	m_Worker_process(4),
	m_Time_resolution(100)
{
	size_t	pos(0);

	initStatusMap();
	contextLines(file, pos);
}

CONF::mainBlock::~mainBlock(void) {}

// public
// const CONF::httpBlock&	CONF::mainBlock::getHttpBlock() {
// 	return ;
// }

const bool	CONF::mainBlock::isDaemonOn() {
	return this->m_Daemon;
}

const unsigned int&	CONF::mainBlock::getWorkerProcess() {
	return this->m_Worker_process;
}

const unsigned long&	CONF::mainBlock::getTimeResolution() {
	return this->m_Time_resolution;
}

const std::vector<std::string>&	CONF::mainBlock::getErrorLog() {
	return this->m_Error_log;
}

const std::string&	CONF::mainBlock::getEnv(const std::string& key) {
	envMap::iterator	it = this->m_Env.find(key);
	if (it != this->m_Env.end()) {
		return it->second;
	} else {
		throw (Utils::errorMessageGenerator(key, 0, "cannot find Environment."));
	}
}

const CONF::mainBlock::envMap&	CONF::mainBlock::getEnvMap() {
	return this->m_Env;
}

const unsigned int&	CONF::mainBlock::getWorkerConnections() {
	return this->m_Event_block.worker_connections;
}


/**
 * @brief	Configure Parser functions
 */

void	CONF::mainBlock::initStatusMap(void) {
	this->m_StatusMap.insert(std::pair<std::string, unsigned char>("env", CONF::E_MAIN_BLOCK_STATUS::ENV));
	this->m_StatusMap.insert(std::pair<std::string, unsigned char>("worker_processes", CONF::E_MAIN_BLOCK_STATUS::WORKER_PROCESS));
	this->m_StatusMap.insert(std::pair<std::string, unsigned char>("daemon", CONF::E_MAIN_BLOCK_STATUS::DAEMON));
	this->m_StatusMap.insert(std::pair<std::string, unsigned char>("timer_resolution", CONF::E_MAIN_BLOCK_STATUS::TIMER_RESOLUTION));
	this->m_StatusMap.insert(std::pair<std::string, unsigned char>("error_log", CONF::E_MAIN_BLOCK_STATUS::ERROR_LOG));
	this->m_StatusMap.insert(std::pair<std::string, unsigned char>("http", CONF::E_MAIN_BLOCK_STATUS::HTTP_BLOCK));
	this->m_StatusMap.insert(std::pair<std::string, unsigned char>("events", CONF::E_MAIN_BLOCK_STATUS::EVENT_BLOCK));
}

unsigned char	CONF::mainBlock::directiveNameChecker(const std::string& name) {
	if (name.empty()) {
		throw (Utils::errorMessageGenerator("", 0, "directive name is empty."));
	}
	const statusMap::iterator it = m_StatusMap.find(name);
	if (it != m_StatusMap.end()) {
		// TODO : 중복 directive일 수도 있어야 함
		(m_Status & it->second) ? throw(Utils::errorMessageGenerator(name, 0, "duplicate directive.")) : m_Status |= it->second;
		return it->second;
	} else {
		throw (Utils::errorMessageGenerator(name, 0, "invalid directive name."));
		// throw ("Test throws");
	}
}

void	CONF::mainBlock::argumentChecker(const std::vector<std::string>& args, const unsigned char& status) {
	switch (status) {
		case CONF::E_MAIN_BLOCK_STATUS::ENV: 
			if (args.size()!= 1) {
				throw (Utils::errorMessageGenerator(args[0], 0, "invalid number of arguments."));
			} else {
				std::pair<std::string, std::string>	env; // = envParser();
				this->m_Env.insert(std::pair<std::string, std::string>(env));
			}
			break;

		case CONF::E_MAIN_BLOCK_STATUS::WORKER_PROCESS: {
			char*	end;
			if (args.size()!= 1) {
				throw (Utils::errorMessageGenerator(args[0], 0, "invalid number of arguments."));
		    } else {
				const long argumentNumber = static_cast<unsigned int>(std::strtol(args[0].c_str(), &end, 10));
				if (*end != '\0' || argumentNumber < 1) {
					throw (Utils::errorMessageGenerator(args[0], 0, "worker_processes argument should not include string or not be negative."));
				}
				argumentNumber > 0? this->m_Worker_process = argumentNumber : throw (Utils::errorMessageGenerator(args[0], 0, "invalid argument."));
			}
			break;

		}
		case CONF::E_MAIN_BLOCK_STATUS::DAEMON:
			if (args.size()!= 1) {
				throw (Utils::errorMessageGenerator(args[0], 0, "invalid number of arguments."));
		    } else {
				if (args[0] == "on") {
					m_Daemon = true;
				} else if (args[0] == "off") {
					;
				} else {
					throw (Utils::errorMessageGenerator(args[0], 0, "invalid argument."));
				}
			}
			break;

		case CONF::E_MAIN_BLOCK_STATUS::TIMER_RESOLUTION: {
			char*	end;
			if (args.size()!= 1) {
				throw (Utils::errorMessageGenerator(args[0], 0, "invalid number of arguments."));
			} else {
                const long argumentNumber = static_cast<unsigned int>(std::strtol(args[0].c_str(), &end, 10));
                argumentNumber > 0? this->m_Time_resolution = argumentNumber : throw (Utils::errorMessageGenerator(args[0], 0, "invalid argument."));
				
				const std::string	time = args[0].substr((end - args[0].c_str()), args[0].length());
				// TODO: time check			
				if (time == "ms") {
					this->m_Time_resolution *= 1000;
				} else if (time == "s" || time.empty()) {
					;
				} else {
					throw (Utils::errorMessageGenerator(args[0], 0, "invalid argument."));
				}
			}
			break;
		}

		case CONF::E_MAIN_BLOCK_STATUS::ERROR_LOG:
			if (args.size()!= 1) {
				throw (Utils::errorMessageGenerator(args[0], 0, "invalid number of arguments."));
			}
			break;

		case CONF::E_MAIN_BLOCK_STATUS::HTTP_BLOCK:
			if (!args.empty()) {
				throw (Utils::errorMessageGenerator(args[0], 0, "invalid number of arguments."));
			}
			break;
			// this->m_Http_block = httpParser();

		case CONF::E_MAIN_BLOCK_STATUS::EVENT_BLOCK:
			if (!args.empty()) {
				throw (Utils::errorMessageGenerator(args[0], 0, "invalid number of arguments."));
			}
			break;
			// this->m_Event_block = eventParser();

		default:
			throw (Utils::errorMessageGenerator(args[0], 0, "invalid directives."));
	}
}

std::string	CONF::mainBlock::argument(const std::string& file, size_t& pos, const unsigned char& status) {
	std::string	argument;
	
	while (pos < file.length() && ABNF::isWSP(file, pos)) {
		pos++;
	}
	switch (status) {
		case CONF::E_MAIN_BLOCK_STATUS::ERROR_LOG: {
			const size_t	argumentLength = (std::strchr(&(file[pos]), E_ABNF::LF) - file.c_str()) - pos - 1;
			const std::string uri = "/" + file.substr(pos, argumentLength);
			size_t	uriPos = 0;
			URIParser::absPath(uri, uriPos, m_Error_log);
			pos += argumentLength + 1;
			break;
		}
		default:
			while (pos < file.size() && (std::isalnum(static_cast<unsigned char>(file[pos])) || file[pos] == '_' || file[pos] == '=')) {
				(std::isalpha(static_cast<unsigned char>(file[pos])) ? argument += std::tolower(file[pos]) : argument += file[pos]);
				pos++;
			}
	}
	return argument;
}

unsigned char	CONF::mainBlock::directiveName(const std::string& file, size_t& pos) {
	std::string	name;

	while (pos < file.size() && (std::isalpha(static_cast<unsigned char>(file[pos])) || file[pos] == '_')) {
		(std::isalpha(static_cast<unsigned char>(file[pos])) ? name += std::tolower(file[pos]) : name += file[pos]);
		pos++;
	}
	return (directiveNameChecker(name));
}

void	CONF::mainBlock::directives(const std::string& file, size_t& pos) {
	const unsigned char		directiveStatus = directiveName(file, pos);
	
	std::vector<std::string>	args;
	while (pos < file.size() && file[pos] != E_ABNF::SEMICOLON && file[pos] != E_ABNF::LF) {
		args.push_back(argument(file, pos, directiveStatus));
	}
	argumentChecker(args, directiveStatus);
}

// bool	CONF::mainBlock::blockContent(const std::string& file, size_t& pos) {

// }

bool	CONF::mainBlock::context(const std::string& file, size_t& pos) {
	if (file[pos] == E_ABNF::SEMICOLON || file[pos] == E_ABNF::LF) {
		return false;
	}
	directives(file, pos);
	ABNF::isC_nl(file, pos);
	// if (ABNF::isC_nl(file, pos)) {
	// if (blockContent(file, pos) || ABNF::isC_nl(file, pos)) {
	// 	return true;
	// } else {
	// 	return false;
	// }
	return true;
}

void	CONF::mainBlock::contextLines(const std::string& file, size_t& pos) {
	while (pos < file.size()) {
		while (pos < file.size() && ABNF::isWSP(file, pos)) {
			pos++;
		}
		context(file, pos) || ABNF::isC_nl(file, pos);
	}
}


/**
 * @brief	Debug
 * 
 */
 void	CONF::mainBlock::print() {
	std::cout << "Main Block" << std::endl;
	std::cout << "\tEnv: " << std::endl;
    for (const auto& env : m_Env) {
		std::cout << "\t\t" << env.first << "=" << env.second << std::endl;
	}
	std::cout << "\tWorker_process: " << m_Worker_process << std::endl;
    std::cout << "\tDaemon: " << (m_Daemon? "on" : "off") << std::endl;
    std::cout << "\tTime_resolution: " << m_Time_resolution << std::endl;
	for (const auto& log : m_Error_log) {
		std::cout << "\tError_log: " << log << std::endl;
	}
 }