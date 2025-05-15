#pragma once
#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

namespace future_test1 {
	int gen_ans(int waittime) {
		static int ans = 0;
		ans += 1;

		std::this_thread::sleep_for(std::chrono::milliseconds(waittime));
		return ans;
	}
	void test() {
		std::future<int> fut = std::async(std::launch::async, gen_ans,5000);

		std::cout << fut.get() << std::endl; // �����ȴ�
		std::cout << "waiting done" << std::endl;
	}
}
namespace future_test2 {
	int gen_ans(int waittime) {
		static int ans = 0;
		ans += 1;

		std::this_thread::sleep_for(std::chrono::milliseconds(waittime));
		return ans;
	}
	void test() {
		std::future<int> fut = std::async(std::launch::deferred, gen_ans, 5000);
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		std::cout << "begin get value" << std::endl;
		std::cout << fut.get() << std::endl; // �����ȴ�

	}
}
namespace future_test3 {
	struct X
	{
		std::string foo(int x, std::string & s) {
			std::cout << "rvalue x=" <<x<< std::endl;
			std::cout << "lvalue s=" <<s<< std::endl;
			s += "1";
			return s;
		}
		std::string foo1(int x, std::string const& s) {
			std::cout << "rvalue x=" << x << std::endl;
			std::cout << "lvalue s=" << s << std::endl;

			return s;
		}
	};

	X x;

	void test() {
		std::string test = "test";
		std::future<std::string> f1 = std::async(&X::foo, &x, 1,ref(test));
		std::cout << f1.get() << std::endl;
		std::cout << test << std::endl;

		//auto f1 = std::async(&X::foo, &x, 42, test);
		//�����ᱨ����Ϊ��std::async ��ʵ���У����ڲ��Ѳ������Ƶ�����һ���߳���ȥִ��
		//���Դ������һ����ֵ 
		//��ֵ�޷����ݸ�����
		//������ֵ���Դ��ݸ��������ã��������µĵ��÷�������

		auto f1 = std::async(&X::foo1, &x, 42, test);


	}

}