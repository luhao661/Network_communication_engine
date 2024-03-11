#pragma once
#include <stdlib.h>
#include <assert.h>//断言库，用于辅助调试程序
class MemoryAlloc;

//内存块 最小单元
class MemoryBlock
{
private:

public:
	//内存块编号
	int nID;

	//引用次数
	int nRef;

	//所属内存块(池)
	MemoryAlloc* pAlloc;

	//下一块位置
	MemoryBlock* pNext;

	//是否在内存池中
	bool bPool;

};
//const int MemoryBlockSize = sizeof(MemoryBlock);//以8字节对齐

//内存池
class MemoryAlloc
{
private:
	//内存池地址
	char* m_pBuf;

	//头部内存单元
	MemoryBlock* pHeader;

	//内存单元的大小
	size_t m_BlockSize;
	//内存单元的数量
	size_t m_BlockNums;

public:
	MemoryAlloc():m_pBuf(nullptr),pHeader(nullptr),m_BlockSize(0), m_BlockNums(0)
	{}
	~MemoryAlloc()
	{}

	//申请内存
	void* allocMem(size_t size)
	{
		return malloc(size);
	}


	//释放内存
	void freeMem(void* p)
	{
		//::free(p);//限定为使用全局命名空间中的函数，若函数名为free，则能防止循环调用

		free(p);
	}

	//初始化内存池
	void initMemory()
	{
		//m_pBuf值必须为nullptr，否则会在标准错误输出上输出诊断信息，并调用 abort()
		assert(nullptr == m_pBuf);
		if (!m_pBuf)
			return;

		//计算内存池的大小
		size_t BuffSize = m_BlockSize * m_BlockNums;//块大小乘以块数量
		//向系统申请池的内存
		m_pBuf = (char*)malloc(BuffSize);

		//初始化内存池
		pHeader = (MemoryBlock*)m_pBuf;//解释为内存块
		pHeader->bPool = true;
		pHeader->nID = 0;
		pHeader->nRef = 0;
		pHeader->pAlloc = this;//所属内存内存块为当前内存池的内存块
		pHeader->pNext = nullptr;

		//将内存池的除开第一块的其他内存块的内存块描述信息都初始化好	
		MemoryBlock* pTmpSlow = pHeader;

		for (size_t n = 1; n < m_BlockSize; ++n)
		{
			MemoryBlock* pTmpFast = (MemoryBlock*)(m_pBuf + (n * m_BlockSize));

			pTmpFast->bPool = true;
			pTmpFast->nID = 0;
			pTmpFast->nRef = 0;
			pTmpFast->pAlloc = this;
			pTmpFast->pNext = nullptr;

			//利用双指针，给pNext成员赋值
			pTmpSlow->pNext = pTmpFast;

			pTmpSlow = pTmpFast;
		}

	}

};


//实现内存池管理
class MemoryMgr
{
private:
	MemoryMgr()
	{}
	~MemoryMgr()
	{}
public:

	//使用单例模式：
	static MemoryMgr& Instance()
	{
		//创建静态对象
		static MemoryMgr mgr;

		return mgr;		
		//《Effective C++》：条款04：确定对象被使用前已先被初始化
		//保证你所获得的那个引用将指向一个历经初始化的对象
	}

	//单例模式：
	// 私有化它的构造函数，以防止外界创建单例类的对象；
	// 	使用类的私有静态指针变量指向类的唯一实例；
	// 使用一个公有的静态方法获取该实例。

	//好处：
	//保证一个类仅有一个实例，并提供一个访问它的全局访问点，
	// 该实例被所有程序模块共享

	//申请内存
	void* allocMem(size_t size)
	{
		return malloc(size);
	}


	//释放内存
	void freeMem(void* p)
	{
		//::free(p);//限定为使用全局命名空间中的函数，若函数名为free，则能防止循环调用

		free(p);
	}
};