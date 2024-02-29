#pragma once

enum
{
	CMD_LOGIN, CMD_LOGIN_RESULT,
	CMD_LOGOUT, CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

//***ע***
//���ٽ���ͷ�Ͱ���ֿ��ýṹ������
//����ͨ���̳У�ֻ��һ�νṹ�壨������󣩾�ͬʱ�õ���ͷ�����

struct DataHead
{
	DataHead()
	{
		datalength = sizeof(DataHead);
		cmd = CMD_ERROR;
	}

	short datalength;
	short cmd;
};

struct LogIn :public DataHead
{
	LogIn()
	{
		datalength = sizeof(LogIn);//100���ֽ�
		cmd = CMD_LOGIN;
	}

	char username[32];
	char password[32];
	char data[32];
};

struct LogInResult :public DataHead
{
	LogInResult()
	{
		datalength = sizeof(LogInResult);//100���ֽ�
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}

	int result;
	char data[92];
};

struct LogOut :public DataHead
{
	LogOut()
	{
		datalength = sizeof(LogOut);
		cmd = CMD_LOGOUT;
	}

	char username[20];
};

struct LogOutResult :public DataHead
{
	LogOutResult()
	{
		datalength = sizeof(LogOutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}

	int result;
};

struct NewUserJoin :public DataHead
{
	NewUserJoin()
	{
		datalength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}

	int sock;
};