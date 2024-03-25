#pragma once

template<typename classT>
class ObjectPoolBase
{
public:
	void* operator new(size_t size)
	{
		return malloc(size);
	}

	void operator delete(void* p)
	{
		delete p;
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

	//使用可变参模板
	template<typename ...Args>
	static classT* CreateObject(Args ...args)
	{
		classT* obj = new classT(args...);

		return obj;
	}

	static void DestoryObject(classT* obj)
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
class ClassB :public ObjectPoolBase<ClassB>
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


