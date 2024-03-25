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
		classT* obj = new classT(n);//ʹ�ù��캯�����г�ʼ��

		//...���������⴦��

		return obj;
	}

	����д�������⣺������Ϊ���࣬
	���ʼ��һ�����󣬵���ͬ���������Ҫ����
	��ͬ���ͺͲ�ͬ������ʵ�Σ���ν����
	*/

	//ʹ�ÿɱ��ģ��
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
ʵ������CreateObject()��װnew���ڳ�ʼ������������ÿɱ��ģ����н����
*/


