#include "pch.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Logger.h"

using namespace std;

int Logger::default_log_level = LOG_LEVEL::LOG_LEVEL_NONE;
std::mutex Logger::mtx;
std::queue<std::string> Logger::log_queue;
std::condition_variable Logger::cv;
bool Logger::terminate_thread = false;
std::thread Logger::log_thread;

Logger::Logger() {}
Logger::~Logger() {}

void Logger::Initialize()
{
    Logger::terminate_thread = false;
    Logger::log_thread = std::thread(&Logger::process_logs);
}

void Logger::shutdown() 
{
    {
        std::lock_guard<std::mutex> lock(Logger::mtx);
        Logger::terminate_thread = true;
    }
    Logger::cv.notify_all();
    if (Logger::log_thread.joinable()) 
    {
        Logger::log_thread.join();
    }
}

string Logger::datetime_now()
{
    time_t now = time(nullptr);
    tm local_time;
    localtime_s(&local_time, &now);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local_time);
    return string(buffer);
}

// Extract file name from a full path
string Logger::get_file_name_from_full_path(const std::string& full_path) 
{
    size_t pos = full_path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return full_path; // directory separator 없으면, return full path
    }
    return full_path.substr(pos + 1);
}

void Logger::write(const string& pgmname, const string& msg)
{
    {
        const std::lock_guard<std::mutex> lock(Logger::mtx);

        string file_name = get_file_name_from_full_path(pgmname);
        string data_time = datetime_now();
        string log_message = file_name + ", " + data_time + ", " + msg + "\n";
        Logger::log_queue.push(log_message);
    }
    cv.notify_one();
}

void Logger::write(std::ostream& os, const std::string& msg, LOG_LEVEL log_level)
{
    {
        const std::lock_guard<std::mutex> lock(Logger::mtx);

        if (log_level >= default_log_level) 
        {
            string data_time = datetime_now();
            os << data_time << " [" << LOG_DESC[log_level] << "] " << msg << std::endl;
        }
    }
}

void Logger::output_to_file(const string& file_name, const string& msg)
{
    ofstream ofs(file_name.c_str(), ios::out | ios::app);
    if (ofs.fail()) 
    {
        cerr << file_name << "couldn't open file!!" << endl;
        throw std::ios_base::failure("Logger::write - " + file_name);
    }
    ofs << msg;
    ofs.clear();
    ofs.close();
}

void Logger::process_logs()
{
    std::chrono::milliseconds log_flush_interval(1000); // 1초
    auto last_flush_time = std::chrono::steady_clock::now();

    while (true)
    {
        std::unique_lock<std::mutex> lock(Logger::mtx);

        // 조건 1 : 큐에 로그가 하나라도 있거나, 종료 요청이 들어오면 스레드가 깨어난다.
        cv.wait_for(lock, log_flush_interval, [] { return !Logger::log_queue.empty() || Logger::terminate_thread; });

        if (Logger::terminate_thread && Logger::log_queue.empty()) break;

        // 조건2 : 버퍼 크기 초과 또는 주기적 타이머
        if (!Logger::log_queue.empty() && (Logger::log_queue.size() >= 10 ||
            std::chrono::steady_clock::now() - last_flush_time >= log_flush_interval)) 
        {
            while (!Logger::log_queue.empty())
            {
                string log_message = Logger::log_queue.front();
                Logger::log_queue.pop();
                lock.unlock();

                string log_file_name = "logfile.log";
                output_to_file(log_file_name, log_message);

                lock.lock();
            }
            last_flush_time = std::chrono::steady_clock::now();
        }
    }
}
