#pragma once
#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include<deque>
#include <condition_variable>
#include <future>
#include<functional>
namespace future_test1 {
	int gen_ans(int waittime) {
		static int ans = 0;
		ans += 1;

		std::this_thread::sleep_for(std::chrono::milliseconds(waittime));
		return ans;
	}
	void test() {
		std::future<int> fut = std::async(std::launch::async, gen_ans,5000);

		std::cout << fut.get() << std::endl; // 阻塞等待
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
		std::cout << fut.get() << std::endl; // 阻塞等待

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
		//这样会报错，因为是std::async 的实现中，他内部把参数复制到另外一个线程中去执行
		//所以传入的是一个右值 
		//右值无法传递给引用
		//但是右值可以传递给常量引用，所以以下的调用方法可行

		auto f2 = std::async(&X::foo1, &x, 42, test);


	}

}
namespace future_test4 {
	//使用future 和 packaged_task 配合主要起到封装函数并取回返回值的作用
	//packaged_task 包装了函数并可以返回future 对象，可以使我们异步获取函数的返回值
	//我们可以异步的执行任务
	std::mutex m;
	std::deque<std::packaged_task<int()>> tasks;
	int test_task() {
		static int i = 0;
		i++;
		return i;
	}
	void gui_thread() {
		while (true)
		{
			std::packaged_task<int()> task; {
				std::lock_guard<std::mutex> lk(m);
				if (tasks.empty()) continue;

				task = std::move(tasks.front());
				tasks.pop_front();
			}
			task();//package_task 需要显式的调用
		}
	}
	template<typename Func>
	std::future<int> post_task(Func f) {
		std::packaged_task<int()> task(f);
		std::future<int> res = task.get_future();
		std::lock_guard<std::mutex> lk(m);
		tasks.push_back(std::move(task));

		return res;
	}

	void test() {
		std::vector<std::future<int>> futurelist;
		std::thread producer(gui_thread);
		for (int i = 0; i < 10; i++) {
			futurelist.push_back(std::move(post_task(test_task)));
		}
		
		for (int i = 0; i < 10; i++) {
			std::cout<<futurelist[i].get()<<std::endl;
		}

		producer.join();
	}
}

namespace future_test5 {
	//promise 和 普通引用相比有如下好处
	//	明确只设置一次值，避免多线程条件竞争问题
	//future.get() 会自动阻塞，直到 promise 设置值；引用做不到
	//具有异常传递能力

	void compute_success(std::promise<int> p) {
		std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟耗时任务
		p.set_value(42);  // 设置结果
	}
	void compute_exception(std::promise<int> p) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		try {
			throw std::runtime_error("Something went wrong!");
		}
		catch (...) {
			p.set_exception(std::current_exception()); // 设置异常
		}
	}

	void test() {
		std::promise<int> promise1;
		std::future<int> future1 = promise1.get_future();

		std::thread t1(compute_success, std::move(promise1));
		t1.detach();
		std::cout << "Waiting for result..." << std::endl;
		future1.wait(); // 如果没有设置结果将会堵塞
		std::cout << future1.get();


		std::promise<int> promise2;
		std::future<int> future2 = promise2.get_future();

		std::thread t2(compute_exception, std::move(promise2));

		try {
			future2.wait();
			int result = future2.get(); // 会抛出异常
			std::cout << "Result from promise2: " << result << std::endl;
		}
		catch (const std::exception& ex) {
			std::cout << "Caught exception from promise2: " << ex.what() << std::endl;
		}
		t2.join();
	
		std::promise<std::string> promise3;
		std::future<std::string> future3 = promise3.get_future();

		std::thread t3([p = std::move(promise3)]() mutable {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			p.set_value("Hello from promise3!");
			});

		std::cout << "Message: " << future3.get() << std::endl;

		t3.join();
	
	}
}

namespace future_test6 {
	//使用future 和 packaged_task 配合主要起到封装函数并取回返回值的作用
	//packaged_task 包装了函数并可以返回future 对象，可以使我们异步获取函数的返回值
	//我们可以异步的执行任务
	std::mutex m;
	std::deque<std::pair<std::function<void(std::promise<int>&)>,std::promise<int>>> tasks;
	static int i = 0;

	void test_task(std::promise<int> &x) {
		i++;
		x.set_value(i);
	}
	void gui_thread() {
		while (true)
		{
			std::function<void(std::promise<int>&)> task;
			std::promise<int> pm;
			 {
				std::lock_guard<std::mutex> lk(m);
				if (tasks.empty()) continue;

				std::tie(task,pm) = std::move(tasks.front());
				tasks.pop_front();
			}
			 task(pm);

		}
	}

	std::future<int> post_task() {
		
		std::promise<int> pm;
		std::future<int> res = pm.get_future();
		//auto lambda_func = [p = std::move(pm)]() mutable{//加mutable 因为被copy 捕获的会被视为const 无法改动
		//	i++;
		//	p.set_value(1);
		//	};//但是不能使用 因为lambda 内的对象要放到function里的话，必须支持拷贝构造
		
		std::lock_guard<std::mutex> lk(m);
		tasks.push_back({test_task,std::move(pm)});

		return res;
	}

	void test() {
		std::vector<std::future<int>> futurelist;
		std::thread producer(gui_thread);
		for (int i = 0; i < 10; i++) {
			futurelist.push_back(std::move(post_task()));
		}

		for (int i = 0; i < 10; i++) {
			std::cout << futurelist[i].get() << std::endl;
		}


		producer.detach();
	}
}