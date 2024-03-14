#pragma once

#if 1
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
protected:
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

	//���ڴ���������ڴ��
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

			//***ע***
			// ��û�������룬�����־��棺C6011:ȡ����NULLָ��"pReturn"������
			if (!pReturn)
			{
				exit(EXIT_FAILURE);
			}

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


	//�ڴ�����ͷ��ڴ��
	//***ע***
	//�������ʵ��ʹ�õ��ڴ��λ��
	void freeMemory(void* pMem)
	{
		//��ȥһ��ƫ������ָ��ǰ��ġ��ڴ��������Ϣ��
		// ����ָ����ϵͳ������ڴ�ġ��ڴ��������Ϣ����
		MemoryBlockMsg* pMemoryBlockMsg =
			(MemoryBlockMsg*)((char*)pMem-sizeof(MemoryBlockMsg));

		assert(1 == pMemoryBlockMsg->nRef);

		//�����ü�����һ
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

		if (m_pBuf)
			return;

		//�����ڴ�صĴ�С
		//����д����
		//size_t BuffSize = m_BlockSize * m_BlockNums;//���С���Կ�����
		size_t BuffSize = (m_BlockSize+sizeof(MemoryBlockMsg)) * m_BlockNums;//���С���Կ�����
		//��ϵͳ����ص��ڴ�
		m_pBuf = (char*)malloc(BuffSize);

		//***ע***
		// ��û�������룬�����־��棺C6011:ȡ����NULLָ��"pHeader"������
		if (!m_pBuf)
		{
			exit(EXIT_FAILURE);
		}
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

		//����д����
		//for (size_t n = 1; n < m_BlockSize; ++n)
		for (size_t n = 1; n < m_BlockNums; ++n)
		{
			//����д����
			//MemoryBlockMsg* pTmpFast = (MemoryBlockMsg*)(m_pBuf + (n * m_BlockSize));

			MemoryBlockMsg* pTmpFast =
				(MemoryBlockMsg*)(m_pBuf + (n * (m_BlockSize + sizeof(MemoryBlockMsg))));

			pTmpFast->bPool = true;
			pTmpFast->nID = n;
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
		//***ע***
		//��MemoryAlloc�е����ݳ�ԱΪprivate����ô���๫�м̳к��޷�����m_BlockSize������
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

	//�����ڴ���Ϊ�ڴ��
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

			if (!pReturn)
			{
				exit(EXIT_FAILURE);
			}

			pReturn->bPool = false;//��ϵͳ������ڴ棬���Բ��������Խ����ڴ����
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;

			//***����д��***
			//return (void*)pReturn;

			return ((char*)pReturn + sizeof(MemoryBlockMsg));
		}
	}

	//�ͷ��ڴ�(pMemָ��ʵ�ʿ��õ��ڴ�)
	void freeMem(void* pMem)
	{
		//��ȥһ��ƫ������ָ��ǰ��ġ��ڴ��������Ϣ��
		// ����ָ����ϵͳ������ڴ�ġ��ڴ��������Ϣ����
		MemoryBlockMsg* pMemoryBlockMsg =
			(MemoryBlockMsg*)((char*)pMem - sizeof(MemoryBlockMsg));

		//�����ڴ�����ڴ����
		if (pMemoryBlockMsg->bPool)
		{
			pMemoryBlockMsg->pAlloc->freeMemory(pMem);
		}
		else
		{
			if (--pMemoryBlockMsg->nRef == 0)
				//����д����
				//free(pMem);
				free(pMemoryBlockMsg);
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
#endif

//
#if 0
#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<assert.h>

#ifdef _DEBUG
#include<stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#else
#define xPrintf(...)
#endif // _DEBUG


#define MAX_MEMORY_SZIE 1024

class MemoryAlloc;
//�ڴ�� ��С��Ԫ
class MemoryBlock
{
public:
	//�������ڴ�飨�أ�
	MemoryAlloc* pAlloc;
	//��һ��λ��
	MemoryBlock* pNext;
	//�ڴ����
	int nID;
	//���ô���
	int nRef;
	//�Ƿ����ڴ����
	bool bPool;
private:
	//Ԥ��
	char c1;
	char c2;
	char c3;
};

//�ڴ��
class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSzie = 0;
		_nBlockSzie = 0;
	}

	~MemoryAlloc()
	{
		if (_pBuf)
			free(_pBuf);
	}

	//�����ڴ�
	void* allocMemory(size_t nSize)
	{
		if (!_pBuf)
		{
			initMemory();
		}

		MemoryBlock* pReturn = nullptr;
		if (nullptr == _pHeader)
		{
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		//xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}

	//�ͷ��ڴ�
	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		if (--pBlock->nRef != 0)
		{
			return;
		}
		if (pBlock->bPool)
		{
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			free(pBlock);
		}
	}

	//��ʼ��
	void initMemory()
	{	//����
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		//�����ڴ�صĴ�С
		size_t realSzie = _nSzie + sizeof(MemoryBlock);
		size_t bufSize = realSzie * _nBlockSzie;
		//��ϵͳ����ص��ڴ�
		_pBuf = (char*)malloc(bufSize);

		//��ʼ���ڴ��
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		//�����ڴ����г�ʼ��
		MemoryBlock* pTemp1 = _pHeader;

		for (size_t n = 1; n < _nBlockSzie; n++)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n * realSzie));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
protected:
	//�ڴ�ص�ַ
	char* _pBuf;
	//ͷ���ڴ浥Ԫ
	MemoryBlock* _pHeader;
	//�ڴ浥Ԫ�Ĵ�С
	size_t _nSzie;
	//�ڴ浥Ԫ������
	size_t _nBlockSzie;
};

//�������������Ա����ʱ��ʼ��MemoryAlloc�ĳ�Ա����
template<size_t nSzie, size_t nBlockSzie>
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//8 4   61/8=7  61%8=5
		const size_t n = sizeof(void*);
		//(7*8)+8 
		_nSzie = (nSzie / n) * n + (nSzie % n ? n : 0);
		_nBlockSzie = nBlockSzie;
	}

};

//�ڴ������
class MemoryMgr
{
private:
	MemoryMgr()
	{
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		init_szAlloc(129, 256, &_mem256);
		init_szAlloc(257, 512, &_mem512);
		init_szAlloc(513, 1024, &_mem1024);
	}

	~MemoryMgr()
	{

	}

public:
	static MemoryMgr& Instance()
	{//����ģʽ ��̬
		static MemoryMgr mgr;
		return mgr;
	}
	//�����ڴ�
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SZIE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else
		{
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			//xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
			return ((char*)pReturn + sizeof(MemoryBlock));
		}

	}

	//�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		//xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else
		{
			if (--pBlock->nRef == 0)
				free(pBlock);
		}
	}

	//�����ڴ������ü���
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//��ʼ���ڴ��ӳ������
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = pMemA;
		}
	}
private:
	MemoryAlloctor<64, 100000> _mem64;
	MemoryAlloctor<128, 100000> _mem128;
	MemoryAlloctor<256, 100000> _mem256;
	MemoryAlloctor<512, 100000> _mem512;
	MemoryAlloctor<1024, 100000> _mem1024;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];
};

#endif // !_MemoryMgr_hpp_

#endif