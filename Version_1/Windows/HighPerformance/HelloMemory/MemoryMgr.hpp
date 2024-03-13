#pragma once
#include <stdlib.h>
#include <assert.h>//���Կ⣬���ڸ������Գ���

//����ڴ���С
#define MAX_MEMORY_SIZE 64

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
//С�ڵ�������ڴ���Сʱ������ʹ���Խ����ڴ��
//��ʼ���ڴ�أ������ڴ棬�ͷ��ڴ�
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
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
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
	//����һ��ÿ���ڴ��64�ֽڣ�����10����ڴ��
	MemoryAllocator<64, 10> m_mem64_10;

	//����һ��ָ�����飬����ӳ�䲻ͬ�ڴ���������¶�Ӧ��Ӧ��ʹ�õ��ڴ��
	MemoryAlloc* m_pMAc[MAX_MEMORY_SIZE + 1];

	MemoryMgr()
	{
		//***���***
		//��0~64�ֽڵ��ڴ���������ڴ��ӳ�䣬ʹ���ڴ������������
		// ���ٵ�ʹ��m_mem64_10�ڴ��
		initMemoryMgr(0,64,&m_mem64_10);
	}

	~MemoryMgr()
	{}

	//�ڴ��ӳ�������ʼ��
	//***���***
	//��һ����Χ�ڵ��ڴ��Сӳ�䵽��Ӧ���ڴ�ء�
	// ��ͨ��ѭ������ָ���ķ�Χ����ÿ���ڴ��С����Ӧ��ָ��ָ����Ӧ���ڴ�ض���
	// ����������Ҫ�����ڴ�ʱ���Ϳ��Ը�����Ҫ�Ĵ�Сֱ���ҵ���Ӧ���ڴ�ض�����з��䡣
	void initMemoryMgr(int nBegin, int nEnd, MemoryAlloc* pMAc)
	{
		for (int n = nBegin; n <= nEnd; ++n)
		{
			//nBegin~nEnd�ֽڷ�Χ��С���ڴ����룬�����Զ�λ��pMAcָ����ڴ��
			m_pMAc[n] = pMAc;

			//***�ô�***
			// allocMem()�п���ֱ��д��
			// return m_pMAc[size]->allocMemory(size);
			//�����˲��Һ��ʵ��ڴ�ص��ڴ���ʱ��
		}
	}

public:

	//ʹ�õ���ģʽ��
	static MemoryMgr& Instance()
	{
		//������̬����(ʹ�����Զ����Ĭ�Ϲ��캯���������)
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
		//���������ڴ�С�ڵ�������ڴ���С
		if (size <= MAX_MEMORY_SIZE)
		{
			return m_pMAc[size]->allocMemory(size);
		}
		else
		{
			//                                                 �ڴ�ռ�+�ڴ��������ϢҪռ�Ŀռ�
			MemoryBlockMsg* pReturn = (MemoryBlockMsg*)malloc(size + sizeof(MemoryBlockMsg));

			pReturn->bPool = false;//��ϵͳ������ڴ棬���Բ��������Խ����ڴ����
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;

			return (void*)pReturn;
		}
	}

	//�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		//��ȥһ��ƫ������ָ��ǰ��ġ��ڴ��������Ϣ��
		// ����ָ����ϵͳ������ڴ�ġ��ڴ��������Ϣ����
		MemoryBlockMsg* pMemoryBlockMsg =
			(MemoryBlockMsg*)((char*)pMem - sizeof(MemoryBlockMsg));

		if (pMemoryBlockMsg->bPool)
		{
			pMemoryBlockMsg->pAlloc->freeMemory(pMem);
		}
		else
		{
			if(--pMemoryBlockMsg->nRef==0)
				free(pMem);
		}

		return;
	}

	//�����ڴ�����ü���(�й����ڴ������ʱ����չ����ʹ��)
	void addRef(void* pMem)
	{
		MemoryBlockMsg* pMemoryBlockMsg =
			(MemoryBlockMsg*)((char*)pMem - sizeof(MemoryBlockMsg));

		++pMemoryBlockMsg->nRef;
	}

};