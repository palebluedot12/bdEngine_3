#pragma once

#include <string>
#include <ostream>
#include <mutex>
#include <queue>

enum LOG_LEVEL 
{
      LOG_LEVEL_NONE
    , LOG_LEVEL_DEBUG
    , LOG_LEVEL_INFORM
    , LOG_LEVEL_WARNING
    , LOG_LEVEL_ERROR
};
const std::string LOG_DESC[]
{
      "DEBUG"
    , "INFORM"
    , "WARNING"
    , "ERROR"
};

/*
    프로그램에 해당하는 파일에 로그 기록
*/
class Logger 
{
public:
    Logger();
    ~Logger();

    static void Initialize();

    static void shutdown();

    static std::string datetime_now();
    static std::string get_file_name_from_full_path(const std::string& full_path);

    // program name, 기록할 내용
    static void write(const std::string& pgmname, const std::string& msg);
    static void write(std::ostream& os, const std::string& msg, LOG_LEVEL log_level = LOG_LEVEL_INFORM);

    // static
    static int default_log_level;
    static std::mutex mtx;
    static std::queue<std::string> log_queue;
    static std::condition_variable cv;
    static bool terminate_thread;
    static std::thread log_thread;

private:
    static void output_to_file(const std::string& file_name, const std::string& msg);

    static void process_logs();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};
