#pragma once

#if 1
#include <stdlib.h>
#include <assert.h>//断言库，用于辅助调试程序

//最大内存块大小
#define MAX_MEMORY_SIZE 64

class MemoryAlloc;

//内存块 最小单元
class MemoryBlockMsg
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
	MemoryBlockMsg* pNext;

	//是否在内存池中
	bool bPool;

};
//const int MemoryBlockSize = sizeof(MemoryBlockMsg);//以8字节对齐


//内存池
//小于等于最大内存块大小时，就能使用自建的内存池
//初始化内存池，申请内存，释放内存
class MemoryAlloc
{
protected:
	//内存池地址
	char* m_pBuf;

	//头部内存单元
	MemoryBlockMsg* pHeader;

	//内存单元的大小
	size_t m_BlockSize;
	//内存单元的数量
	size_t m_BlockNums;

public:
	MemoryAlloc():m_pBuf(nullptr),pHeader(nullptr),m_BlockSize(0), m_BlockNums(0)
	{}

	~MemoryAlloc()
	{
		if (m_pBuf)
			free(m_pBuf);
	}

	//在内存池中申请内存块
	void* allocMemory(size_t size)
	{
		if (!m_pBuf)
		{
			initMemory();
		}

		MemoryBlockMsg* pReturn = nullptr;
		
		//内存池的内存空间不足时
		if (nullptr == pHeader)
		{ 
			//                                                 内存空间+内存块描述信息要占的空间
			pReturn = (MemoryBlockMsg*)malloc(size+sizeof(MemoryBlockMsg));

			//***注***
			// 若没有这块代码，则会出现警告：C6011:取消对NULL指针"pReturn"的引用
			if (!pReturn)
			{
				exit(EXIT_FAILURE);
			}

			pReturn->bPool = false;//向系统申请的内存，所以不在我们自建的内存池中
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else
		{
			//使用pHeader指向的内存块
			pReturn = pHeader;

			//pHeader指向位置后移，指向新的可被使用的内存块
			pHeader = pHeader->pNext;

			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		//错误：
		//return pReturn;

		return ((char*)pReturn + sizeof(MemoryBlockMsg));
		//理解：
		//返回一个偏移地址。因为pReturn原指向一个内存块中的MemoryBlockMsg内容
		//现在加上一个偏移量后，指向【实际可用内存】
	}


	//内存池中释放内存块
	//***注***
	//传入的是实际使用的内存的位置
	void freeMemory(void* pMem)
	{
		//减去一个偏移量后，指向当前块的【内存块描述信息】
		// （或指向向系统申请的内存的【内存块描述信息】）
		MemoryBlockMsg* pMemoryBlockMsg =
			(MemoryBlockMsg*)((char*)pMem-sizeof(MemoryBlockMsg));

		assert(1 == pMemoryBlockMsg->nRef);

		//将引用计数减一
		if (--pMemoryBlockMsg->nRef != 0)
		{
			return;//引用值大于1，说明有共享内存的情况
		}
		
		//若要释放的内存块在内存池内
		if (pMemoryBlockMsg->bPool)
		{
			//理解：
			//申请内存时，有如下代码：
			/*
			//使用pHeader指向的内存块
			pReturn = pHeader;
			//pHeader指向位置后移，指向新的可被使用的内存块
			pHeader = pHeader->pNext;
			*/

			//那么释放内存块时，当前块的下一块【可用】内存应该正由pHeader所指向
			pMemoryBlockMsg->pNext = pHeader;
			//pHeader回过来指向当前可被存储数据的可用内存块的内存块信息
			pHeader = pMemoryBlockMsg;
		}
		else//若在内存池外
		{
			free(pMemoryBlockMsg);
		}

		return;
	}

	//初始化内存池
	void initMemory()
	{
		//m_pBuf值必须为nullptr，否则会在标准错误输出上输出诊断信息，并调用 abort()
		assert(nullptr == m_pBuf);

		if (m_pBuf)
			return;

		//计算内存池的大小
		//错误写法：
		//size_t BuffSize = m_BlockSize * m_BlockNums;//块大小乘以块数量
		size_t BuffSize = (m_BlockSize+sizeof(MemoryBlockMsg)) * m_BlockNums;//块大小乘以块数量
		//向系统申请池的内存
		m_pBuf = (char*)malloc(BuffSize);

		//***注***
		// 若没有这块代码，则会出现警告：C6011:取消对NULL指针"pHeader"的引用
		if (!m_pBuf)
		{
			exit(EXIT_FAILURE);
		}
		//内存池的第一块内存块的描述信息赋好值
		//***注***
		//语句"pHeader = (MemoryBlockMsg*)m_pBuf;//解释为内存块"
		//该块内存区域从左往右，内容为【内存块描述信息】和【实际可用内存】
		pHeader = (MemoryBlockMsg*)m_pBuf;//解释为内存块
		pHeader->bPool = true;
		pHeader->nID = 0;
		pHeader->nRef = 0;
		pHeader->pAlloc = this;//所属内存内存块为当前内存池的内存块
		pHeader->pNext = nullptr;

		//将内存池的除开第一块的其他内存块的内存块描述信息都赋值好	
		MemoryBlockMsg* pTmpSlow = pHeader;

		//错误写法：
		//for (size_t n = 1; n < m_BlockSize; ++n)
		for (size_t n = 1; n < m_BlockNums; ++n)
		{
			//错误写法：
			//MemoryBlockMsg* pTmpFast = (MemoryBlockMsg*)(m_pBuf + (n * m_BlockSize));

			MemoryBlockMsg* pTmpFast =
				(MemoryBlockMsg*)(m_pBuf + (n * (m_BlockSize + sizeof(MemoryBlockMsg))));

			pTmpFast->bPool = true;
			pTmpFast->nID = n;
			pTmpFast->nRef = 0;
			pTmpFast->pAlloc = this;
			pTmpFast->pNext = nullptr;

			//利用双指针，给pNext成员赋值
			pTmpSlow->pNext = pTmpFast;

			pTmpSlow = pTmpFast;
		}

	}

};


//不直接在MemoryMgr类中创建MemoryAlloc对象
//创建的MemoryAlloc对象无法更改m_BlockSize、m_BlockNums参数
//使用模板元编程(TMP)
//将工作从运行期转移到编译期，并且可以指定m_BlockSize、m_BlockNums参数
template<size_t BlockSize,size_t BlockNums>//注意此处传的是非模板类型参数
class MemoryAllocator :public MemoryAlloc
{
public:
	MemoryAllocator()
	{
		//如果传入m_BlockSize为61，就要处理成【内存对齐】的合适的值
		const size_t n = sizeof(void*);

		//m_BlockSize = BlockSize;

		//(61/8)*8+(61%8)=56+8=64
		m_BlockSize = (BlockSize / n) * n + (BlockSize%n ? n : 0);
		m_BlockNums = BlockNums;
		//***注***
		//若MemoryAlloc中的数据成员为private，那么该类公有继承后将无法访问m_BlockSize等数据
	}
};


//内存池管理工具
class MemoryMgr
{
private:
	//申请一个每个内存块64字节，共有10块的内存池
	MemoryAllocator<64, 10> m_mem64_10;

	//创建一个指针数组，用于映射不同内存需求情况下对应的应该使用的内存池
	MemoryAlloc* m_pMAc[MAX_MEMORY_SIZE + 1];

	MemoryMgr()
	{
		//***理解***
		//对0~64字节的内存申请进行内存块映射，使该内存申请操作可以
		// 快速地使用m_mem64_10内存池
		initMemoryMgr(0,64,&m_mem64_10);
	}

	~MemoryMgr()
	{}

	//内存池映射数组初始化
	//***理解***
	//将一定范围内的内存大小映射到对应的内存池。
	// 它通过循环遍历指定的范围，将每个内存大小所对应的指针指向相应的内存池对象。
	// 这样，当需要分配内存时，就可以根据需要的大小直接找到对应的内存池对象进行分配。
	void initMemoryMgr(int nBegin, int nEnd, MemoryAlloc* pMAc)
	{
		for (int n = nBegin; n <= nEnd; ++n)
		{
			//nBegin~nEnd字节范围大小的内存申请，都可以定位到pMAc指向的内存池
			m_pMAc[n] = pMAc;

			//***好处***
			// allocMem()中可以直接写：
			// return m_pMAc[size]->allocMemory(size);
			//减少了查找合适的内存池的内存块的时间
		}
	}

public:

	//使用单例模式：
	static MemoryMgr& Instance()
	{
		//创建静态对象(使用了自定义的默认构造函数构造对象)
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

	//申请内存作为内存池
	void* allocMem(size_t size)
	{
		//如果申请的内存小于等于最大内存块大小
		if (size <= MAX_MEMORY_SIZE)
		{
			return m_pMAc[size]->allocMemory(size);
		}
		else
		{
			//                                                 内存空间+内存块描述信息要占的空间
			MemoryBlockMsg* pReturn = (MemoryBlockMsg*)malloc(size + sizeof(MemoryBlockMsg));

			if (!pReturn)
			{
				exit(EXIT_FAILURE);
			}

			pReturn->bPool = false;//向系统申请的内存，所以不在我们自建的内存池中
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;

			//***错误写法***
			//return (void*)pReturn;

			return ((char*)pReturn + sizeof(MemoryBlockMsg));
		}
	}

	//释放内存(pMem指向实际可用的内存)
	void freeMem(void* pMem)
	{
		//减去一个偏移量后，指向当前块的【内存块描述信息】
		// （或指向向系统申请的内存的【内存块描述信息】）
		MemoryBlockMsg* pMemoryBlockMsg =
			(MemoryBlockMsg*)((char*)pMem - sizeof(MemoryBlockMsg));

		//若该内存块在内存池中
		if (pMemoryBlockMsg->bPool)
		{
			pMemoryBlockMsg->pAlloc->freeMemory(pMem);
		}
		else
		{
			if (--pMemoryBlockMsg->nRef == 0)
				//错误写法：
				//free(pMem);
				free(pMemoryBlockMsg);
		}

		return;
	}

	//增加内存块引用计数(有共享内存块需求时做扩展方法使用)
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
//内存块 最小单元
class MemoryBlock
{
public:
	//所属大内存块（池）
	MemoryAlloc* pAlloc;
	//下一块位置
	MemoryBlock* pNext;
	//内存块编号
	int nID;
	//引用次数
	int nRef;
	//是否在内存池中
	bool bPool;
private:
	//预留
	char c1;
	char c2;
	char c3;
};

//内存池
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

	//申请内存
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

	//释放内存
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

	//初始化
	void initMemory()
	{	//断言
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		//计算内存池的大小
		size_t realSzie = _nSzie + sizeof(MemoryBlock);
		size_t bufSize = realSzie * _nBlockSzie;
		//向系统申请池的内存
		_pBuf = (char*)malloc(bufSize);

		//初始化内存池
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		//遍历内存块进行初始化
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
	//内存池地址
	char* _pBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存单元的大小
	size_t _nSzie;
	//内存单元的数量
	size_t _nBlockSzie;
};

//便于在声明类成员变量时初始化MemoryAlloc的成员数据
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

//内存管理工具
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
	{//单例模式 静态
		static MemoryMgr mgr;
		return mgr;
	}
	//申请内存
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

	//释放内存
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

	//增加内存块的引用计数
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//初始化内存池映射数组
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