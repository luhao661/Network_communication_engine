#pragma once

#include <stdlib.h>
#include <assert.h>
#include <mutex>

//错误写法：会出现重定义
#if 0
//在debug模式下添加能输出调试信息的输出语句
#ifdef _DEBUG
//使用变参宏
#include <stdio.h>
								//记：Variable Arguments
#define xPrintf(...) printf(__VA_ARGS__)

//在非debug模式下，xPrintf(...) 替换成无操作
#else
#define xPrintf(...) 
#endif
#endif

//正确写法：
//在debug模式下添加能输出调试信息的输出语句
#ifdef _DEBUG

#ifndef xPrintf
		//使用变参宏
		#include <stdio.h>
										//记：Variable Arguments
		#define xPrintf(...) printf(__VA_ARGS__)
#endif

//在非debug模式下，xPrintf(...) 替换成无操作
#else

#ifndef xPrintf
#define xPrintf(...) 
#endif

#endif

template<class ClassT, size_t ObjectNums>
class CellObjectPool
{
private:
	class NodeMsg//对象池内存块描述信息
	{
	private:
		char c1;
		char c2;//刚好16字节填满
	public:
		//是否在对象池中
		bool bPool;
		//内存块编号
		int nID;
		//引用次数
		char nRef;
		//所属内存块(池)
		//NodeMsg* pAlloc;
		//下一块位置
		NodeMsg* pNext;
	};

	//对象池地址
	char* m_pBuf;

	//头部内存单元
	NodeMsg* pHeader;

	//内存单元的大小
	//size_t m_BlockSize;
	//内存单元的数量
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

	//申请对象所要占用的对象池内存
	void* allocObjMemory(size_t size)
	{
		std::lock_guard<std::mutex> lg(m_mutex);

		//if (!m_pBuf)
		//{
		//	initMemory();
		//}

		NodeMsg* pReturn = nullptr;

		//对象池的内存空间不足时
		if (nullptr == pHeader)
		{
			//                                                 内存空间+内存块描述信息要占的空间
			//pReturn = (MemoryBlockMsg*)malloc(size + sizeof(MemoryBlockMsg));
			pReturn = (NodeMsg*)new char [sizeof(NodeMsg) + sizeof(ClassT)];

			//***注***
			// 若没有这块代码，则会出现警告：C6011:取消对NULL指针"pReturn"的引用
			if (!pReturn)
			{
				exit(EXIT_FAILURE);
			}

			pReturn->bPool = false;//向自建的内存池申请的内存
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else
		{
			//使用pHeader指向的对象池内存块
			pReturn = pHeader;

			//pHeader指向位置后移，指向新的可被使用的对象池内存块
			pHeader = pHeader->pNext;

			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		xPrintf("allocObjMemory: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, size);

		//错误：
		//return pReturn;
		return ((char*)pReturn + sizeof(NodeMsg));
	}

	//释放对象所占用的对象池内存
	void freeObjMemory(void* pMem)
	{
		//减去一个偏移量后，指向当前块的【内存块描述信息】
		NodeMsg* pNodeMsg =
			(NodeMsg*)((char*)pMem - sizeof(NodeMsg));

		assert(1 == pNodeMsg->nRef);

		xPrintf("freeObjMem: %llx, id=%d\n", pNodeMsg, pNodeMsg->nID);

		//若要释放的内存块在对象池内
		if (pNodeMsg->bPool)
		{
			std::lock_guard<std::mutex> lg(m_mutex);

			//将引用计数减一
			if (--pNodeMsg->nRef != 0)
			{
				return;//引用值大于1，说明有共享内存的情况
			}

			//理解：
			//申请内存时，有如下代码：
			/*
			//使用pHeader指向的内存块
			pReturn = pHeader;
			//pHeader指向位置后移，指向新的可被使用的内存块
			pHeader = pHeader->pNext;
			*/
			//那么释放内存块时，当前块的下一块【可用】内存应该正由pHeader所指向
			pNodeMsg->pNext = pHeader;
			//pHeader回过来指向当前可被存储数据的可用内存块的内存块信息
			pHeader = pNodeMsg;
		}
		else//若在对象池外
		{
			//std::lock_guard<std::mutex> lg(m_mutex);

			if (--pNodeMsg->nRef != 0)
			{
				return;//引用值大于1，说明有共享内存的情况
			}

			//free(pNodeMsg);
			delete[] pNodeMsg;
		}

		return;
	}

private:
	//初始化对象池
	void initObjectPool()
	{
		//断言
		assert(nullptr == m_pBuf);

		if (m_pBuf)
			return;

		//计算对象池大小
		size_t RealBlockSize = sizeof(NodeMsg) + sizeof(ClassT);
		//***补充***
		//相比于创建内存池，创建对象不用进行内存对齐计算，因为该处已经
		//交给类型判断，自动对齐内存了
		size_t RealPoolSize = ObjectNums * RealBlockSize;
		//优先向自建的内存池去申请内存
		m_pBuf = new char[RealPoolSize];

		//初始化内存池
		pHeader = (NodeMsg*)m_pBuf;//解释为对象池内存块描述信息
		pHeader->bPool = true;
		pHeader->nID = 0;
		pHeader->nRef = 0;
		pHeader->pNext = nullptr;

		//将内存池的除开第一块的其他内存块的内存块描述信息都赋值好	
		NodeMsg* pTmpSlow = pHeader;

		for (size_t n = 1; n < ObjectNums; ++n)
		{
			NodeMsg* pTmpFast =
				(NodeMsg*)(m_pBuf + (n * RealBlockSize));

			pTmpFast->bPool = true;
			pTmpFast->nID = n;
			pTmpFast->nRef = 0;
			pTmpFast->pNext = nullptr;

			//利用双指针，给pNext成员赋值
			pTmpSlow->pNext = pTmpFast;

			pTmpSlow = pTmpFast;
		}	

	}
};


template<typename ClassT,size_t ObjectNums>
class ObjectPoolBase
{
private:

	//对传入模板的实际class类型，取别名为ClassTPool
	//加上“,size_t ObjectNums”
	//可以在创建实际类时这样写：class ClassB :public ObjectPoolBase<ClassB,10> {};
	typedef CellObjectPool<ClassT, ObjectNums> ClassTPool;

	//创建对象池单例
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
		classT* obj = new classT(n);//使用构造函数进行初始化

		//...（可做额外处理）

		return obj;
	}

	以上写法的问题：该类作为基类，
	想初始化一个对象，但不同对象可能需要传入
	不同类型和不同个数的实参，如何解决？
	*/

	//使用可变参模板，兼容不同类对象初始化时的参数类型和个数的要求
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

	//补充：(《Effective C++》)
	/*
	使用static的原因是：
	类初始化早于类实例化，非静态资源是在实例化后才存在于内存，
	静态资源是在类初始化后就存在于静态存储区，在类中不占用内存

	类外函数不能通过类名来调用类的普通成员函数，
	但可以调用调用类的静态成员函数

	能通过类对象来调用类的普通成员函数和静态成员函数

	因为静态成员函数无this指针，所以不能访问非静态成员

	静态方法只能引用静态资源
	非静态方法可以引用非静态资源和静态资源
	*/
};

//该类的使用方法：
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
实现了用CreateObject()封装new，在初始化对象操作上用可变参模板进行解耦和
*/


