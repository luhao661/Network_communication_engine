#pragma once

#include"CELL.hpp"
#include"Client.hpp"

namespace engine {
	namespace io {
		//自定义
		class Server;

		//网络事件接口
		class INetEvent
		{
		public:
			//纯虚函数
			//客户端加入事件
			virtual void OnNetJoin(Client* pClient) = 0;
			//客户端离开事件
			virtual void OnNetLeave(Client* pClient) = 0;
			//客户端消息事件
			virtual void OnNetMsg(Server* pServer, Client* pClient, netmsg_DataHeader* header) = 0;
			//recv事件
			virtual void OnNetRecv(Client* pClient) = 0;
		private:

		};
	}
}
