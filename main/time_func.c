#include <stdio.h>
#include <time.h>
#include "esp_sntp.h"

#include "time_func.h"


static void obtain_time(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    time_t now = 0;
    struct tm timeinfo = { 0 };
    while (timeinfo.tm_year < (2020 - 1900)) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

// Function to get the current timestamp in ISO format
void get_current_timestamp(char *timestamp, size_t timestamp_size) {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime); // Get current time
    timeinfo = localtime(&rawtime); // Convert to local time

    // Format the time to ISO 8601 format: YYYY-MM-DDTHH:MM:SSZ
    strftime(timestamp, timestamp_size, "%Y-%m-%dT%H:%M:%SZ", timeinfo);
}