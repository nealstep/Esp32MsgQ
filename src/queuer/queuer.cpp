#include "queuer.hpp"

#ifdef ARDUINO

bool Queuer::push(Queues qs, const Modules::Reading reading) {
    QueueHandle_t* q;
    q = _get_queue(qs);
    if (q && (xQueueSend(*q, &reading, qwait) == pdPASS)) {
        return true;
    }
    return false;
}

bool Queuer::pop(Queues qs, Modules::Reading& reading) {
    QueueHandle_t* q;
    q = _get_queue(qs);
    if (q && (xQueueReceive(*q, &reading, 0) == pdPASS)) {
        return true;
    }
    return false;
}

#else

bool Queuer::push(Queues qs, const Modules::Reading reading) {
    readings.push(reading);
    return true;
}

bool Queuer::pop(Queues qs, Modules::Reading& reading) {
    bool ret = !readings.empty();
    if (ret) {
        reading = readings.front();
        readings.pop();
    }
    return ret;
}

#endif  // ARDUINO !ARDUINO