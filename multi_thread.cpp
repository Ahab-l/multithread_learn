// multi_thread.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <thread>
#include <chrono>

void worker() {
    for (int i = 0; i < 5; ++i) {
        std::cout << "子线程工作中: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "子线程完成\n";
}
void test() {
    std::thread t(worker);
    t.detach();  // 主线程不再等待子线程
}
void test2(int& x) {
    std::cout << x;
}
int main() {
    test2(int(2));
    while (true)
    {

    }
    std::cout << "主线程即将结束\n";

    std::cout << "主线程结束\n";
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
