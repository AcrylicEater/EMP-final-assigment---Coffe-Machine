#include "coffee_control.h"

uint32_t get_runtime(){
    return xTaskGetTickCount() / configTICK_RATE_HZ;
}

timestamp_t get_timestamp(){
    uint32_t all_seconds = get_runtime(); //get total amount of run time seconds since

    if(all_seconds > SEC_IN_DAY){  //we only need the time of day, if we run for more than one day
        all_seconds -= SEC_IN_DAY; //remove the extra day
    }

    timestamp_t return_val;
    return_val.sec = all_seconds % TIME_BASE;
    return_val.min = (all_seconds / TIME_BASE) % TIME_BASE;
    return_val.hr  = all_seconds / (TIME_BASE*TIME_BASE);

    return return_val;
}
