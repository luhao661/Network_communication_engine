#pragma once

#if _WIN32
	#include"TcpSelectServer.hpp"

//后续支持
#elif __linux__
	#include"TcpEpollServer.hpp"
#else
	#include"TcpSelectServer.hpp"
#endif

namespace engine {
	namespace io {
#if _WIN32
		typedef TcpSelectServer TcpServerMgr;
#elif __linux__
		typedef TcpEpollServer TcpServerMgr;
#else
		typedef TcpSelectServer TcpServerMgr;
#endif
	}
}
