#pragma once

#include <stdlib.h>
#include <assert.h>
#include <mutex>

//����д����������ض���
#if 0
//��debugģʽ����������������Ϣ��������
#ifdef _DEBUG
//ʹ�ñ�κ�
#include <stdio.h>
								//�ǣ�Variable Arguments
#define xPrintf(...) printf(__VA_ARGS__)

//�ڷ�debugģʽ�£�xPrintf(...) �滻���޲���
#else
#define xPrintf(...) 
#endif
#endif

//��ȷд����
//��debugģʽ����������������Ϣ��������
#ifdef _DEBUG

#ifndef xPrintf
		//ʹ�ñ�κ�
		#include <stdio.h>
										//�ǣ�Variable Arguments
		#define xPrintf(...) printf(__VA_ARGS__)
#endif

//�ڷ�debugģʽ�£�xPrintf(...) �滻���޲���
#else

#ifndef xPrintf
#define xPrintf(...) 
#endif

#endif

template<class ClassT, size_t ObjectNums>
class CellObjectPool
{
private:
	class NodeMsg//������ڴ��������Ϣ
	{
	private:
		char c1;
		char c2;//�պ�16�ֽ�����
	public:
		//�Ƿ��ڶ������
		bool bPool;
		//�ڴ����
		int nID;
		//���ô���
		char nRef;
		//�����ڴ��(��)
		//NodeMsg* pAlloc;
		//��һ��λ��
		NodeMsg* pNext;
	};

	//����ص�ַ
	char* m_pBuf;

	//ͷ���ڴ浥Ԫ
	NodeMsg* pHeader;

	//�ڴ浥Ԫ�Ĵ�С
	//size_t m_BlockSize;
	//�ڴ浥Ԫ������
	//size_t m_BlockNums;

	std::mutex m_mutex;

public:
	CellObjectPool()
	{
		initObjectPool();
	}
	
	~CellObjectPool()
	{
		if (m_pBuf)
			delete[] m_pBuf;
	}

	//���������Ҫռ�õĶ�����ڴ�
	void* allocObjMemory(size_t size)
	{
		std::lock_guard<std::mutex> lg(m_mutex);

		//if (!m_pBuf)
		//{
		//	initMemory();
		//}

		NodeMsg* pReturn = nullptr;

		//����ص��ڴ�ռ䲻��ʱ
		if (nullptr == pHeader)
		{
			//                                                 �ڴ�ռ�+�ڴ��������ϢҪռ�Ŀռ�
			//pReturn = (MemoryBlockMsg*)malloc(size + sizeof(MemoryBlockMsg));
			pReturn = (NodeMsg*)new char [sizeof(NodeMsg) + sizeof(ClassT)];

			//***ע***
			// ��û�������룬�����־��棺C6011:ȡ����NULLָ��"pReturn"������
			if (!pReturn)
			{
				exit(EXIT_FAILURE);
			}

			pReturn->bPool = false;//���Խ����ڴ��������ڴ�
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else
		{
			//ʹ��pHeaderָ��Ķ�����ڴ��
			pReturn = pHeader;

			//pHeaderָ��λ�ú��ƣ�ָ���µĿɱ�ʹ�õĶ�����ڴ��
			pHeader = pHeader->pNext;

			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		xPrintf("allocObjMemory: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, size);

		//����
		//return pReturn;
		return ((char*)pReturn + sizeof(NodeMsg));
	}

	//�ͷŶ�����ռ�õĶ�����ڴ�
	void freeObjMemory(void* pMem)
	{
		//��ȥһ��ƫ������ָ��ǰ��ġ��ڴ��������Ϣ��
		NodeMsg* pNodeMsg =
			(NodeMsg*)((char*)pMem - sizeof(NodeMsg));

		assert(1 == pNodeMsg->nRef);

		xPrintf("freeObjMem: %llx, id=%d\n", pNodeMsg, pNodeMsg->nID);

		//��Ҫ�ͷŵ��ڴ���ڶ������
		if (pNodeMsg->bPool)
		{
			std::lock_guard<std::mutex> lg(m_mutex);

			//�����ü�����һ
			if (--pNodeMsg->nRef != 0)
			{
				return;//����ֵ����1��˵���й����ڴ�����
			}

			//��⣺
			//�����ڴ�ʱ�������´��룺
			/*
			//ʹ��pHeaderָ����ڴ��
			pReturn = pHeader;
			//pHeaderָ��λ�ú��ƣ�ָ���µĿɱ�ʹ�õ��ڴ��
			pHeader = pHeader->pNext;
			*/
			//��ô�ͷ��ڴ��ʱ����ǰ�����һ�顾���á��ڴ�Ӧ������pHeader��ָ��
			pNodeMsg->pNext = pHeader;
			//pHeader�ع���ָ��ǰ�ɱ��洢���ݵĿ����ڴ����ڴ����Ϣ
			pHeader = pNodeMsg;
		}
		else//���ڶ������
		{
			//std::lock_guard<std::mutex> lg(m_mutex);

			if (--pNodeMsg->nRef != 0)
			{
				return;//����ֵ����1��˵���й����ڴ�����
			}

			//free(pNodeMsg);
			delete[] pNodeMsg;
		}

		return;
	}

private:
	//��ʼ�������
	void initObjectPool()
	{
		//����
		assert(nullptr == m_pBuf);

		if (m_pBuf)
			return;

		//�������ش�С
		size_t RealBlockSize = sizeof(NodeMsg) + sizeof(ClassT);
		//***����***
		//����ڴ����ڴ�أ����������ý����ڴ������㣬��Ϊ�ô��Ѿ�
		//���������жϣ��Զ������ڴ���
		size_t RealPoolSize = ObjectNums * RealBlockSize;
		//�������Խ����ڴ��ȥ�����ڴ�
		m_pBuf = new char[RealPoolSize];

		//��ʼ���ڴ��
		pHeader = (NodeMsg*)m_pBuf;//����Ϊ������ڴ��������Ϣ
		pHeader->bPool = true;
		pHeader->nID = 0;
		pHeader->nRef = 0;
		pHeader->pNext = nullptr;

		//���ڴ�صĳ�����һ��������ڴ����ڴ��������Ϣ����ֵ��	
		NodeMsg* pTmpSlow = pHeader;

		for (size_t n = 1; n < ObjectNums; ++n)
		{
			NodeMsg* pTmpFast =
				(NodeMsg*)(m_pBuf + (n * RealBlockSize));

			pTmpFast->bPool = true;
			pTmpFast->nID = n;
			pTmpFast->nRef = 0;
			pTmpFast->pNext = nullptr;

			//����˫ָ�룬��pNext��Ա��ֵ
			pTmpSlow->pNext = pTmpFast;

			pTmpSlow = pTmpFast;
		}	

	}
};


template<typename ClassT,size_t ObjectNums>
class ObjectPoolBase
{
private:

	//�Դ���ģ���ʵ��class���ͣ�ȡ����ΪClassTPool
	//���ϡ�,size_t ObjectNums��
	//�����ڴ���ʵ����ʱ����д��class ClassB :public ObjectPoolBase<ClassB,10> {};
	typedef CellObjectPool<ClassT, ObjectNums> ClassTPool;

	//��������ص���
	static ClassTPool& ObjectPool()
	{
		static ClassTPool  COP;

		return COP;
	}

public:
	void* operator new(size_t size)
	{
		//return malloc(size);

		return ObjectPool().allocObjMemory(size);
	}

	void operator delete(void* p)
	{
		//delete p;

		ObjectPool().freeObjMemory(p);
	}

	/*
	static classT* CreateObject(int n)
	{
		classT* obj = new classT(n);//ʹ�ù��캯�����г�ʼ��

		//...���������⴦��

		return obj;
	}

	����д�������⣺������Ϊ���࣬
	���ʼ��һ�����󣬵���ͬ���������Ҫ����
	��ͬ���ͺͲ�ͬ������ʵ�Σ���ν����
	*/

	//ʹ�ÿɱ��ģ�壬���ݲ�ͬ������ʼ��ʱ�Ĳ������ͺ͸�����Ҫ��
	template<typename ...Args>
	static ClassT* CreateObject(Args ...args)
	{
		ClassT* obj = new ClassT(args...);

		return obj;
	}

	static void DestoryObject(ClassT* obj)
	{
		delete obj;
	}

	//���䣺(��Effective C++��)
	/*
	ʹ��static��ԭ���ǣ�
	���ʼ��������ʵ�������Ǿ�̬��Դ����ʵ������Ŵ������ڴ棬
	��̬��Դ�������ʼ����ʹ����ھ�̬�洢���������в�ռ���ڴ�

	���⺯������ͨ�����������������ͨ��Ա������
	�����Ե��õ�����ľ�̬��Ա����

	��ͨ������������������ͨ��Ա�����;�̬��Ա����

	��Ϊ��̬��Ա������thisָ�룬���Բ��ܷ��ʷǾ�̬��Ա

	��̬����ֻ�����þ�̬��Դ
	�Ǿ�̬�����������÷Ǿ�̬��Դ�;�̬��Դ
	*/
};

//�����ʹ�÷�����
/*
class ClassB :public ObjectPoolBase<ClassB,10>
{
private:
	int num = 0;

public:
	ClassB(int n, int m)
	{
		num = n + m;
	}

	~ClassB()
	{

	}
};
ʵ������CreateObject()��װnew���ڳ�ʼ������������ÿɱ��ģ����н����
*/


