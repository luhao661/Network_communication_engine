#pragma once
#include <stdlib.h>
#include <assert.h>//���Կ⣬���ڸ������Գ���
class MemoryAlloc;

//�ڴ�� ��С��Ԫ
class MemoryBlockMsg
{
private:

public:
	//�ڴ����
	int nID;

	//���ô���
	int nRef;

	//�����ڴ��(��)
	MemoryAlloc* pAlloc;

	//��һ��λ��
	MemoryBlockMsg* pNext;

	//�Ƿ����ڴ����
	bool bPool;

};
//const int MemoryBlockSize = sizeof(MemoryBlockMsg);//��8�ֽڶ���


//�ڴ��
class MemoryAlloc
{
private:
	//�ڴ�ص�ַ
	char* m_pBuf;

	//ͷ���ڴ浥Ԫ
	MemoryBlockMsg* pHeader;

	//�ڴ浥Ԫ�Ĵ�С
	size_t m_BlockSize;
	//�ڴ浥Ԫ������
	size_t m_BlockNums;

public:
	MemoryAlloc():m_pBuf(nullptr),pHeader(nullptr),m_BlockSize(0), m_BlockNums(0)
	{}
	~MemoryAlloc()
	{
		if (m_pBuf)
			free(m_pBuf);
	}

	//�����ڴ�
	void* allocMemory(size_t size)
	{
		if (!m_pBuf)
		{
			initMemory();
		}

		MemoryBlockMsg* pReturn = nullptr;
		
		//�ڴ�ص��ڴ�ռ䲻��ʱ
		if (nullptr == pHeader)
		{ 
			//                                                 �ڴ�ռ�+�ڴ��������ϢҪռ�Ŀռ�
			pReturn = (MemoryBlockMsg*)malloc(size+sizeof(MemoryBlockMsg));

			pReturn->bPool = false;//��ϵͳ������ڴ棬���Բ��������Խ����ڴ����
			pReturn->nID = -1;
			pReturn->nRef = 0;
			pReturn->pAlloc = this;
			pReturn->pNext = nullptr;
		}
		else
		{
			//ʹ��pHeaderָ����ڴ��
			pReturn = pHeader;

			//pHeaderָ��λ�ú��ƣ�ָ���µĿɱ�ʹ�õ��ڴ��
			pHeader = pHeader->pNext;

			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		//����
		//return pReturn;

		return ((char*)pReturn + sizeof(MemoryBlockMsg));
		//��⣺
		//����һ��ƫ�Ƶ�ַ����ΪpReturnԭָ��һ���ڴ���е�MemoryBlockMsg����
		//���ڼ���һ��ƫ������ָ��ʵ�ʿ����ڴ桿
	}


	//�ͷ��ڴ�
	//***ע***
	//�������ʵ��ʹ�õ��ڴ��λ��
	void freeMemory(void* pMem)
	{
		//��ȥһ��ƫ������ָ��ǰ��ġ��ڴ��������Ϣ��
		// ����ָ����ϵͳ������ڴ�ġ��ڴ��������Ϣ����
		MemoryBlockMsg* pMemoryBlockMsg =
			(MemoryBlockMsg*)((char*)pMem-sizeof(MemoryBlockMsg));

		assert(1 == pMemoryBlockMsg->nRef);

		if (--pMemoryBlockMsg->nRef != 0)
		{
			return;//����ֵ����1��˵���й����ڴ�����
		}
		
		//��Ҫ�ͷŵ��ڴ�����ڴ����
		if (pMemoryBlockMsg->bPool)
		{
			//��⣺
			//�����ڴ�ʱ�������´��룺
			/*
			//ʹ��pHeaderָ����ڴ��
			pReturn = pHeader;
			//pHeaderָ��λ�ú��ƣ�ָ���µĿɱ�ʹ�õ��ڴ��
			pHeader = pHeader->pNext;
			*/
			//��ô�ͷ��ڴ��ʱ����ǰ�����һ�顾���á��ڴ�Ӧ������pHeader��ָ��
			pMemoryBlockMsg->pNext = pHeader;
			//pHeader�ع���ָ��ǰ�ɱ��洢���ݵĿ����ڴ����ڴ����Ϣ
			pHeader = pMemoryBlockMsg;
		}
		else//�����ڴ����
		{
			free(pMemoryBlockMsg);
		}

		return;
	}

	//��ʼ���ڴ��
	void initMemory()
	{
		//m_pBufֵ����Ϊnullptr��������ڱ�׼�����������������Ϣ�������� abort()
		assert(nullptr == m_pBuf);
		if (!m_pBuf)
			return;

		//�����ڴ�صĴ�С
		size_t BuffSize = m_BlockSize * m_BlockNums;//���С���Կ�����
		//��ϵͳ����ص��ڴ�
		m_pBuf = (char*)malloc(BuffSize);

		//�ڴ�صĵ�һ���ڴ���������Ϣ����ֵ
		//***ע***
		//���"pHeader = (MemoryBlockMsg*)m_pBuf;//����Ϊ�ڴ��"
		//�ÿ��ڴ�����������ң�����Ϊ���ڴ��������Ϣ���͡�ʵ�ʿ����ڴ桿
		pHeader = (MemoryBlockMsg*)m_pBuf;//����Ϊ�ڴ��
		pHeader->bPool = true;
		pHeader->nID = 0;
		pHeader->nRef = 0;
		pHeader->pAlloc = this;//�����ڴ��ڴ��Ϊ��ǰ�ڴ�ص��ڴ��
		pHeader->pNext = nullptr;

		//���ڴ�صĳ�����һ��������ڴ����ڴ��������Ϣ����ֵ��	
		MemoryBlockMsg* pTmpSlow = pHeader;

		for (size_t n = 1; n < m_BlockSize; ++n)
		{
			MemoryBlockMsg* pTmpFast = (MemoryBlockMsg*)(m_pBuf + (n * m_BlockSize));

			pTmpFast->bPool = true;
			pTmpFast->nID = 0;
			pTmpFast->nRef = 0;
			pTmpFast->pAlloc = this;
			pTmpFast->pNext = nullptr;

			//����˫ָ�룬��pNext��Ա��ֵ
			pTmpSlow->pNext = pTmpFast;

			pTmpSlow = pTmpFast;
		}

	}

};


//��ֱ����MemoryMgr���д���MemoryAlloc����
//������MemoryAlloc�����޷�����m_BlockSize��m_BlockNums����
//ʹ��ģ��Ԫ���(TMP)
//��������������ת�Ƶ������ڣ����ҿ���ָ��m_BlockSize��m_BlockNums����
template<size_t BlockSize,size_t BlockNums>//ע��˴������Ƿ�ģ�����Ͳ���
class MemoryAllocator :public MemoryAlloc
{
public:
	MemoryAllocator()
	{
		//�������m_BlockSizeΪ61����Ҫ����ɡ��ڴ���롿�ĺ��ʵ�ֵ
		const size_t n = sizeof(void*);

		//m_BlockSize = BlockSize;

		//(61/8)*8+(61%8)=56+8=64
		m_BlockSize = (BlockSize / n) * n + (BlockSize%n ? n : 0);
		m_BlockNums = BlockNums;
	}
};


//�ڴ�ع�����
class MemoryMgr
{
private:
	MemoryAllocator<64, 10> m_mem64_10;

	MemoryMgr()
	{}
	~MemoryMgr()
	{}
public:

	//ʹ�õ���ģʽ��
	static MemoryMgr& Instance()
	{
		//������̬����
		static MemoryMgr mgr;

		return mgr;		
		//��Effective C++��������04��ȷ������ʹ��ǰ���ȱ���ʼ��
		//��֤������õ��Ǹ����ý�ָ��һ��������ʼ���Ķ���
	}

	//����ģʽ��
	// ˽�л����Ĺ��캯�����Է�ֹ��紴��������Ķ���
	// 	ʹ�����˽�о�ָ̬�����ָ�����Ψһʵ����
	// ʹ��һ�����еľ�̬������ȡ��ʵ����

	//�ô���
	//��֤һ�������һ��ʵ�������ṩһ����������ȫ�ַ��ʵ㣬
	// ��ʵ�������г���ģ�鹲��

	//�����ڴ�
	void* allocMem(size_t size)
	{
		return malloc(size);
	}


	//�ͷ��ڴ�
	void freeMem(void* p)
	{
		//::free(p);//�޶�Ϊʹ��ȫ�������ռ��еĺ�������������Ϊfree�����ܷ�ֹѭ������

		free(p);
	}
};