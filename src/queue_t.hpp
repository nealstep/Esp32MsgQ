#pragma once

#include "global.hpp"

#ifndef ARDUINO
#include <queue>
#endif  // !ARDUINO

template <typename T>
class QueueT {
   public:
#ifdef ARDUINO
    bool Queuer::push(const T& entry) {
        if (xQueueSend(_queue, entry, qwait) == pdPASS) {
            return true;
        }
        return false;
    }

    bool Queuer::pop(T& entry) {
        if (xQueueReceive(_queue, entry, 0) == pdPASS) {
            return true;
        }
        return false;
    }
#else
    bool push(const T& entry) {
        _queue.push(entry);
        return false;
    }
    bool pop(T& entry) {
        bool ret = !_queue.empty();
        if (ret) {
            entry = _queue.front();
            _queue.pop();
        }
        return ret;
    }
#endif  // ARDUINO !ARDUINO

   protected:
#ifdef ARDUINO
    QueueHandle_t _queue;
#else
    std::queue<T> _queue;
#endif  // ARDUINO !ARDUINO
};
