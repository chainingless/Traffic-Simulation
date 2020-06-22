#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return !_queue.empty(); });
    
    T msg = std::move(_queue.back());
    _queue.pop_back();
    
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */
 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while (true) {
        TrafficLightPhase lightPhase = _messages.receive();
        if (lightPhase == green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{   
    std::chrono::time_point<std::chrono::system_clock> lastUpdateTime = std::chrono::system_clock::now();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        long duriationSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdateTime).count();

        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(4, 6);
        int maxDuriation = distribution(generator);
        
        if (duriationSinceLastUpdate >= maxDuriation) {
            switch (_currentPhase) {
                case green:
                    _currentPhase = red;
                    break;
                case red:
                    _currentPhase = green;
                    break;
            }
            _messages.send(std::move(_currentPhase));
            lastUpdateTime = std::chrono::system_clock::now();
        }
    }
}

