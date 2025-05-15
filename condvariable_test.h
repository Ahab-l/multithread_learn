#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
//条件变量的定义
//若条件变量与某一事件或某一条件关联，一个或多个线程就能以其为依托，等待条件成立。当某线程判定条件成立时，就通过该条件变量
// ，知会所有等待的线程，唤醒它们继续处理。
namespace cv_test1 {
    int queuesize = 1000000;
    // 模拟数据结构
    struct data_chunk {
        int value;
    };

    // 模拟是否还有数据要准备
    bool more_data_to_prepare() {
        static int count = 0;
        return count++ < queuesize; // 准备10次数据
    }

    // 模拟准备数据
    data_chunk prepare_data() {
        static int val = 1;
        return data_chunk{ val++ };
    }

    // 模拟判断是否是最后一块数据
    bool is_last_chunk(const data_chunk& data) {
        return data.value == queuesize;
    }

    // 模拟处理数据
    void process(const data_chunk& data) {
        std::cout << "Processing data: " << data.value << std::endl;
    }

    // 全局共享资源
    std::mutex mut;
    std::queue<data_chunk> data_queue;
    std::condition_variable data_cond;
    auto lfunc = []() {
        if (data_queue.empty()) {
            std::cout << "队列为空,继续等待" << std::endl;
            return false;
        }
        else {
            std::cout << "队列非空,有"<<data_queue.size()<<"，消费" << std::endl;
            return true;
        }
    };


    // 线程乙：生产者线程
    void data_preparation_thread() {
        while (more_data_to_prepare()) {
            data_chunk const data = prepare_data();
            {
                std::lock_guard<std::mutex> lk(mut); // ② 加锁保护共享队列

                data_queue.push(data);
            }
            data_cond.notify_one(); // 通知条件变量，并使线程的阻塞解除
        }
    }

    // 线程甲：消费者线程
    void data_processing_thread() {
        while (true) {
            std::unique_lock<std::mutex> lk(mut); // ④ 加锁并关联 condition_variable

            // ⑤ 条件变量等待队列非空（防止虚假唤醒）
			//convariable 在判断前回先占据互斥，lambda 为真 则占据互斥并继续执行，后面手动释放互斥
			// lambda函数返回 false 时，条件变量会自动释放互斥锁，并自动堵塞
            data_cond.wait(lk,lfunc);

            data_chunk data = data_queue.front(); // 访问数据
            data_queue.pop();                     // 弹出数据
            lk.unlock();                          // ⑥ 尽早释放锁

            process(data);
            if (is_last_chunk(data))
                break;
        }
    }

	void test() {
        std::thread consumer(data_processing_thread);
        std::thread producer(data_preparation_thread);

        consumer.join(); // 等待消费者线程结束
        producer.join(); // 等待生产者线程结束

	}

}
namespace cv_test2 {
    //连续唤醒测试
    std::mutex mut;
	std::condition_variable data_cond;
    bool notified=false;//如果不使用 notified 以及相关的逻辑判断，就会导致伪唤醒
    void notifier_func() {
        while (true)
        {
            {
				std::lock_guard<std::mutex> lk(mut);
				std::cout << "random_Notifying..." << std::endl;
                notified = true;
            }
            data_cond.notify_one();

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
        }
    }
    void listener_func(int x) {
		while (true)
		{
			std::unique_lock<std::mutex> lk(mut);
			
			data_cond.wait(lk, [] { 
                if (notified) {
                    notified = false;
                    return true;
                }
                else return false; });
			std::cout <<"listener:"<<x<< " Notified!" << std::endl;

            lk.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
    }
	void test() {
        std::thread notifier(notifier_func);
        std::thread listener(listener_func,1);
        std::thread listener1(listener_func, 2);
        std::thread listener2(listener_func, 3);
        std::thread listener3(listener_func, 4);
        std::thread listener4(listener_func, 5);

		notifier.join(); // 等待消费者线程结束
		listener.join(); // 等待生产者线程结束
        listener1.join(); // 等待生产者线程结束
        listener2.join(); // 等待生产者线程结束
        listener3.join(); // 等待生产者线程结束
        listener4.join(); // 等待生产者线程结束 
	}
}

