#include <stdio.h>
#include <stdlib.h>
#include "serialio.h"
#include "protocol.h"

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Please pass the serial port via command line.\n Example: \"uart-tester.exe COM3\" or \"./uart-tester /dev/tty/USB0\"");
        return -1;
    }
    /** Open serial port **/
    if(!serial_open(argv[1]))
    {
        fprintf(stderr, "Cannot open");
        return -1;
    }

    /** send request **/
    request request = {
        .direct_voltage = UP_MEASURE_RANGE_AUTO,
        .direct_current = UP_MEASURE_RANGE_AUTO,
        .alternating_voltage = UP_MEASURE_RANGE_AUTO,
        .alternating_current = UP_MEASURE_RANGE_AUTO,
        .resistance = UP_MEASURE_RANGE_AUTO,
        .frequency = UP_MEASURE_RANGE_AUTO,
        .duty_cycle = UP_MEASURE_RANGE_AUTO,
    };
    protocol_send_request(&request);

    /** read response **/
    response response;
    if(!protocol_wait_response(&response))
    {
        fprintf(stderr, "Response error!");
        return -1;
    }
    protocol_print_response(&response);

    /** close serial port **/
    if(!serial_close())
    {
        fprintf(stderr, "Cannot close");
        return -1;
    }
    return 0;
}
