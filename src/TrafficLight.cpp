#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mtx);

    _condition.wait(uLock, [this]{ return !this->_queue.empty(); });

    T message = std::move(_queue.front());
    _queue.pop_front();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> ulock(_mtx);
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
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto trafficPhase = _messageQueue.receive();
        if(trafficPhase == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
     threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(4000,6000);
    int haltTimeRandom = distribution(generator);
    std::chrono::milliseconds currentTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
    std::chrono::milliseconds pastTime = currentTime;
    std::chrono::milliseconds haltTime = std::chrono::milliseconds(haltTimeRandom);
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::chrono::milliseconds timeDifference = currentTime - pastTime;
        //std::cout << "time difference = " << timeDifference.count() << std::endl;
        if (timeDifference > haltTime) {
            //std::cout << "inside the if .........thread id " << std::this_thread::get_id() << std::endl;
            //std::cout << "timeDifference" << timeDifference .count() << std::endl;
            this->_currentPhase = this->_currentPhase == TrafficLightPhase::red? TrafficLightPhase::green : TrafficLightPhase::red;
            pastTime = currentTime;
            haltTimeRandom = distribution(generator);
            haltTime = std::chrono::milliseconds(haltTimeRandom);
            //std::cout << "new halttime" << haltTime.count()  << std::endl;
            _messageQueue.send(std::move(this->_currentPhase));
        }
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
    }

}

