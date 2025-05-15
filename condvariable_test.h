#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
//���������Ķ���
//������������ĳһ�¼���ĳһ����������һ�������߳̾�������Ϊ���У��ȴ�������������ĳ�߳��ж���������ʱ����ͨ������������
// ��֪�����еȴ����̣߳��������Ǽ�������
namespace cv_test1 {
    int queuesize = 1000000;
    // ģ�����ݽṹ
    struct data_chunk {
        int value;
    };

    // ģ���Ƿ�������Ҫ׼��
    bool more_data_to_prepare() {
        static int count = 0;
        return count++ < queuesize; // ׼��10������
    }

    // ģ��׼������
    data_chunk prepare_data() {
        static int val = 1;
        return data_chunk{ val++ };
    }

    // ģ���ж��Ƿ������һ������
    bool is_last_chunk(const data_chunk& data) {
        return data.value == queuesize;
    }

    // ģ�⴦������
    void process(const data_chunk& data) {
        std::cout << "Processing data: " << data.value << std::endl;
    }

    // ȫ�ֹ�����Դ
    std::mutex mut;
    std::queue<data_chunk> data_queue;
    std::condition_variable data_cond;
    auto lfunc = []() {
        if (data_queue.empty()) {
            std::cout << "����Ϊ��,�����ȴ�" << std::endl;
            return false;
        }
        else {
            std::cout << "���зǿ�,��"<<data_queue.size()<<"������" << std::endl;
            return true;
        }
    };


    // �߳��ң��������߳�
    void data_preparation_thread() {
        while (more_data_to_prepare()) {
            data_chunk const data = prepare_data();
            {
                std::lock_guard<std::mutex> lk(mut); // �� ���������������

                data_queue.push(data);
            }
            data_cond.notify_one(); // ֪ͨ������������ʹ�̵߳��������
        }
    }

    // �̼߳ף��������߳�
    void data_processing_thread() {
        while (true) {
            std::unique_lock<std::mutex> lk(mut); // �� ���������� condition_variable

            // �� ���������ȴ����зǿգ���ֹ��ٻ��ѣ�
			//convariable ���ж�ǰ����ռ�ݻ��⣬lambda Ϊ�� ��ռ�ݻ��Ⲣ����ִ�У������ֶ��ͷŻ���
			// lambda�������� false ʱ�������������Զ��ͷŻ����������Զ�����
            data_cond.wait(lk,lfunc);

            data_chunk data = data_queue.front(); // ��������
            data_queue.pop();                     // ��������
            lk.unlock();                          // �� �����ͷ���

            process(data);
            if (is_last_chunk(data))
                break;
        }
    }

	void test() {
        std::thread consumer(data_processing_thread);
        std::thread producer(data_preparation_thread);

        consumer.join(); // �ȴ��������߳̽���
        producer.join(); // �ȴ��������߳̽���

	}

}
namespace cv_test2 {
    //�������Ѳ���
    std::mutex mut;
	std::condition_variable data_cond;
    bool notified=false;//�����ʹ�� notified �Լ���ص��߼��жϣ��ͻᵼ��α����
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

		notifier.join(); // �ȴ��������߳̽���
		listener.join(); // �ȴ��������߳̽���
        listener1.join(); // �ȴ��������߳̽���
        listener2.join(); // �ȴ��������߳̽���
        listener3.join(); // �ȴ��������߳̽���
        listener4.join(); // �ȴ��������߳̽��� 
	}
}

