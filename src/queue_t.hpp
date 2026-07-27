#pragma once

#include "global.hpp"

#ifndef ARDUINO
#include <queue>
#endif  // !ARDUINO

template <typename T>
class QueueT {
   public:
#ifdef ARDUINO
    static constexpr const uint8_t queue_size = 10;
    static constexpr const TickType_t push_wait = 3;
    static constexpr const TickType_t pop_wait = 0;

    QueueT(uint8_t qsize = queue_size) {
        _queue = xQueueCreate(qsize, sizeof(T));
    }

    bool push(const T& entry) {
        if (xQueueSend(_queue, &entry, push_wait) == errQUEUE_FULL) {
            // drop last element in queue (imitate circular)
            T bogus;
            xQueueReceive(_queue, &bogus, pop_wait);
            if (xQueueSend(_queue, &entry, push_wait) == errQUEUE_FULL) {
                return false;
            }
        }
        return false;
    }

    bool pop(T& entry) {
        if (xQueueReceive(_queue, &entry, pop_wait) == pdPASS) {
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
