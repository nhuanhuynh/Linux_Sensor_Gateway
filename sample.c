#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <string>
#include <chrono>

// Shared structure
class SharedDataQueue {
private:
    queue<string> dataQueue; // Queue để lưu dữ liệu từ sensor
    mutex mtx;                   // Mutex để bảo vệ queue
    condition_variable cv;       // Condition variable để đồng bộ
    bool done = false;                // Tín hiệu kết thúc

public:
    // Thêm dữ liệu vào queue
    void push(const string& data)
    {
        lock_guard<mutex> lock(mtx);
        dataQueue.push(data);
        cv.notify_one(); // Thông báo cho thread xử lý
    }

    // Lấy dữ liệu từ queue
    bool pop(string& data) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]() { return !dataQueue.empty() || done; });
        
        if (!dataQueue.empty()) {
            data = dataQueue.front();
            dataQueue.pop();
            return true;
        }

        return false;
    }

    // Đánh dấu hoàn thành
    void set_done() {
        lock_guard<mutex> lock(mtx);
        done = true;
        cv.notify_all(); // Thông báo cho tất cả thread
    }

    bool is_done() {
        lock_guard<mutex> lock(mtx);
        return done && dataQueue.empty();
    }
};

// Thread đọc dữ liệu từ sensor qua TCP client
void tcp_client_thread(SharedDataQueue& sharedQueue) {
    int count = 0;
    while (count < 10) { // Giả lập đọc dữ liệu từ sensor
        this_thread::sleep_for(chrono::milliseconds(500)); // Giả lập thời gian đọc
        string data = "SensorData_" + to_string(++count);
        cout << "TCP Client: Received " << data << endl;
        sharedQueue.push(data);
    }
    sharedQueue.set_done();
}

// Thread xử lý dữ liệu
void processing_thread(SharedDataQueue& sharedQueue) {
    string data;
    while (sharedQueue.pop(data)) {
        cout << "Processing: Handling " << data << endl;
        this_thread::sleep_for(chrono::milliseconds(300)); // Giả lập thời gian xử lý
    }
    cout << "Processing: All data processed. Exiting..." << endl;
}

int main() {
    SharedDataQueue sharedQueue;

    // Tạo hai thread: một đọc dữ liệu, một xử lý dữ liệu
    thread tcpClient(tcp_client_thread, ref(sharedQueue));
    thread processor(processing_thread, ref(sharedQueue));

    // Chờ cả hai thread hoàn thành
    tcpClient.join();
    processor.join();

    return 0;
}
