#include "tacothreadpool.hpp"

#include <thread>

namespace taco {
    ThreadPool thrpool(std::thread::hardware_concurrency());
}