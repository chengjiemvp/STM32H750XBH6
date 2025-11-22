#pragma once
#include "main.h"

namespace bsp_setup {
    struct RTCDateTime {
        RTC_TimeTypeDef time;
        RTC_DateTypeDef date;
    };
    RTCDateTime rtc_setup();
}
