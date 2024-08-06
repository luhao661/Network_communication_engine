#pragma once

#include"TcpServerMgr.hpp"
#include"HttpClient.hpp"

namespace engine {
	namespace io {
		class TcpHttpServer :public TcpServerMgr
		{
			virtual Client* makeClientObj(SOCKET cSock)
			{
				return new HttpClient(cSock, _nSendBuffSize, _nRecvBuffSize);
			}
		};
	}
}
