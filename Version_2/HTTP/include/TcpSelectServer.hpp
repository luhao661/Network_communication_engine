#pragma once

#include"TcpServer.hpp" 
#include"SelectServer.hpp"
#include"FDSet.hpp"
namespace engine {
	namespace io {
		class TcpSelectServer : public TcpServer
		{
		public:
			void Start(int nCELLServer)
			{
				TcpServer::Start<SelectServer>(nCELLServer);
			}
		protected:
			//处理网络消息
			void OnRun(Thread* pThread)
			{
				//伯克利套接字 BSD socket
				//描述符（socket） 集合
				FDSet fdRead;
				fdRead.create(_nMaxClient);
				while (pThread->isRun())
				{
					time4msg();
					//清理集合
					fdRead.zero();
					//将描述符（socket）加入集合
					fdRead.add(sockfd());

					timeval t = { 0, 1 };
					int ret = select(sockfd() + 1, fdRead.fdset(), 0, 0, &t); 
					if (ret < 0)
					{
						if (errno == EINTR)
						{
							CELLLog_Info("TcpSelectServer select EINTR");
							continue;
						}

						CELLLog_PError("TcpSelectServer.OnRun select.");
						pThread->Exit();
						break;
					}
					//判断描述符（socket）是否在集合中
					if (fdRead.has(sockfd()))
					{
						//fdRead.del(_sock);
						Accept();
					}
				}
			}
		};
	}
}
