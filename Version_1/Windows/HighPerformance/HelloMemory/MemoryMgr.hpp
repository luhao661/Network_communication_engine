#pragma once
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
private:
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

	//申请内存
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


	//释放内存
	//***注***
	//传入的是实际使用的内存的位置
	void freeMemory(void* pMem)
	{
		//减去一个偏移量后，指向当前块的【内存块描述信息】
		// （或指向向系统申请的内存的【内存块描述信息】）
		MemoryBlockMsg* pMemoryBlockMsg =
			(MemoryBlockMsg*)((char*)pMem-sizeof(MemoryBlockMsg));

		assert(1 == pMemoryBlockMsg->nRef);

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
		if (!m_pBuf)
			return;

		//计算内存池的大小
		size_t BuffSize = m_BlockSize * m_BlockNums;//块大小乘以块数量
		//向系统申请池的内存
		m_pBuf = (char*)malloc(BuffSize);

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

		for (size_t n = 1; n < m_BlockSize; ++n)
		{
			MemoryBlockMsg* pTmpFast = (MemoryBlockMsg*)(m_pBuf + (n * m_BlockSize));

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

	//申请内存
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

			pReturn->bPool = false;//向系统申请的内存，所以不在我们自建的内存池中
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;

			return (void*)pReturn;
		}
	}

	//释放内存
	void freeMem(void* pMem)
	{
		//减去一个偏移量后，指向当前块的【内存块描述信息】
		// （或指向向系统申请的内存的【内存块描述信息】）
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

	//增加内存块引用计数(有共享内存块需求时做扩展方法使用)
	void addRef(void* pMem)
	{
		MemoryBlockMsg* pMemoryBlockMsg =
			(MemoryBlockMsg*)((char*)pMem - sizeof(MemoryBlockMsg));

		++pMemoryBlockMsg->nRef;
	}

};