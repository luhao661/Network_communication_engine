#pragma once
#include <stdlib.h>
#include <assert.h>//���Կ⣬���ڸ������Գ���
class MemoryAlloc;

//�ڴ�� ��С��Ԫ
class MemoryBlock
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
	MemoryBlock* pNext;

	//�Ƿ����ڴ����
	bool bPool;

};
//const int MemoryBlockSize = sizeof(MemoryBlock);//��8�ֽڶ���

//�ڴ��
class MemoryAlloc
{
private:
	//�ڴ�ص�ַ
	char* m_pBuf;

	//ͷ���ڴ浥Ԫ
	MemoryBlock* pHeader;

	//�ڴ浥Ԫ�Ĵ�С
	size_t m_BlockSize;
	//�ڴ浥Ԫ������
	size_t m_BlockNums;

public:
	MemoryAlloc():m_pBuf(nullptr),pHeader(nullptr),m_BlockSize(0), m_BlockNums(0)
	{}
	~MemoryAlloc()
	{}

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

		//��ʼ���ڴ��
		pHeader = (MemoryBlock*)m_pBuf;//����Ϊ�ڴ��
		pHeader->bPool = true;
		pHeader->nID = 0;
		pHeader->nRef = 0;
		pHeader->pAlloc = this;//�����ڴ��ڴ��Ϊ��ǰ�ڴ�ص��ڴ��
		pHeader->pNext = nullptr;

		//���ڴ�صĳ�����һ��������ڴ����ڴ��������Ϣ����ʼ����	
		MemoryBlock* pTmpSlow = pHeader;

		for (size_t n = 1; n < m_BlockSize; ++n)
		{
			MemoryBlock* pTmpFast = (MemoryBlock*)(m_pBuf + (n * m_BlockSize));

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


//ʵ���ڴ�ع���
class MemoryMgr
{
private:
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