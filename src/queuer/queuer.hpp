#pragma once

#include "global.hpp"
#include "modules/modules.hpp"

#ifdef ARDUINO
#else
#include <queue>
#endif  // ARDUINO !ARDIUNO

class Queuer {
   public:
    enum Queues : uint8_t { Rdgs };

    // lazy singleton
    static Queuer& getInstance(void) {
        static Queuer instance;
        return instance;
    }
    Queuer(const Queuer&) = delete;
    Queuer& operator=(const Queuer&) = delete;

#ifdef ARDUINO
    QueueHandle_t readings;

    bool push(Queues qs, const Modules::Reading reading);
    bool pop(Queues qs, Modules::Reading& reading);
#else
    std::queue<Modules::Reading> readings;

    bool push(Queues qs, const Modules::Reading reading);
    bool pop(Queues qs, Modules::Reading& reading);
#endif  // ARDUINO !ARDUINO

   protected:
#ifdef ARDUINO
    static constexpr uint8_t qwait = 10;
#endif  // ARDUINO

    // hidden creator
    Queuer(void) {
#ifdef ARDUINO
        readings = xQueueCreate(10, sizeof(Modules::Reading));
#endif  // ARDUINO
    };

#ifdef ARDUINO
    QueueHandle_t* _get_queue(Queues qs) {
        switch (qs) {
            case Queues::Rdgs:
                return &readings;
            default:
                // this should be impossible
                return nullptr;
                break;
        }
    }
#endif  // ARDUINO
};

static Queuer& queuer = Queuer::getInstance();
