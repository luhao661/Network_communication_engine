#include"TcpSelectServer.hpp"
#include"Config.hpp"

using namespace engine::io;

class MyServer : public TcpSelectServer
{
public:
	MyServer()
	{
		_bSendBack = Config::Instance().hasKey("-sendback");
		_bSendFull = Config::Instance().hasKey("-sendfull");
		_bCheckMsgID = Config::Instance().hasKey("-checkMsgID");
	}

	virtual void OnNetJoin(Client* pClient)
	{
		TcpServer::OnNetJoin(pClient);
	}

	virtual void OnNetLeave(Client* pClient)
	{
		TcpServer::OnNetLeave(pClient);
	}
  
	virtual void OnNetMsg(Server* pServer, Client* pClient, netmsg_DataHeader* header)
	{
		TcpServer::OnNetMsg(pServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTHeart();
			netmsg_Login* login = (netmsg_Login*)header;
			//检查消息ID
			if (_bCheckMsgID)
			{ 
				if (login->msgID != pClient->nRecvMsgID)
				{
					//若当前消息ID和本地收消息次数不匹配
					CELLLog_Error("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d> %d", pClient->sockfd(), login->msgID, pClient->nRecvMsgID, login->msgID - pClient->nRecvMsgID);
				}
				++pClient->nRecvMsgID;
			}

			if (_bSendBack)
			{
				netmsg_LoginR ret;
				ret.msgID = pClient->nSendMsgID;
				if (SOCKET_ERROR == pClient->SendData(&ret))
				{
					//发送缓冲区满了，消息没发出去
					if (_bSendFull)
					{
						CELLLog_Warring("<Socket=%d> Send Full", pClient->sockfd());
					}
				}
				else
				{
					++pClient->nSendMsgID;
				}
			}

		}
		break;
		case CMD_LOGOUT:
		{
			pClient->resetDTHeart();
		}
		break;
		case CMD_C2S_HEART:
		{
			pClient->resetDTHeart();
			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
		}
		default:
		{
			CELLLog_Info("recv <socket=%d> undefine msgType,dataLen：%d", pClient->sockfd(), header->dataLength);
		}
		break;
		}
	}
private:
	//自定义标志 收到消息后将返回应答消息
	bool _bSendBack;
	//自定义标志 是否提示：发送缓冲区已写满
	bool _bSendFull;
	//是否检查接收到的消息ID是否连续
	bool _bCheckMsgID;
};

int main(int argc, char* args[])
{
	//设置运行日志名称
	Log::Instance().setLogPath("serverLog", "w", false);
	Config::Instance().Init(argc, args);

	const char* strIP = Config::Instance().getStr("strIP", "any");
	uint16_t nPort = Config::Instance().getInt("nPort", 9190);
	int nThread = Config::Instance().getInt("nThread", 1);

	if (Config::Instance().hasKey("-p"))
	{
		CELLLog_Info("hasKey -p");
	}

	if (strcmp(strIP, "any") == 0)
	{
		strIP = nullptr;
	}

	MyServer server;
	if (Config::Instance().hasKey("-ipv6"))
	{
		server.InitSocket(AF_INET6);
	}
	else 
	{
		server.InitSocket();
	}
	server.Bind(strIP, nPort);
	server.Listen(SOMAXCONN);
	server.Start(nThread);

	//在主线程中等待用户输入命令
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			break;
		}
		else
		{
			CELLLog_Info("undefine cmd");
		}
	}

	CELLLog_Info("exit.");
	return 0;
}
