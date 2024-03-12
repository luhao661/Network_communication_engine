#pragma once
#include <stdlib.h>
#include <assert.h>//断言库，用于辅助调试程序
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
			pReturn->nRef = 0;
			pReturn->pAlloc = this;
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
	MemoryAllocator<64, 10> m_mem64_10;

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