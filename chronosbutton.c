/* Standard Linux Headers */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
//#include <string.h>

/* Local includes */
#include "debug.h"

/* Function prototypes */

void signal_handler(int sig);
void cleanup(int status);

/* Global variable to register quit */
int quit = 0;


/**********************************************
 * signal_handler
 *
 * Code to execute upon pressing Ctrl-C
 **********************************************/
void signal_handler(int sig) {
    DBG("Ctrl-C pressed, cleaning up and exiting...\n");
    quit = 1;
}


/**********************************************
 * main
 *
 * start of program
 **********************************************/
int main(int argc, char *argv[]) {
    int status = EXIT_SUCCESS;

    /* Set the signal callback for Ctrl-C */
    signal(SIGINT, signal_handler);
    DBG("Ctrl-C handler set.\n");

    /* Open file descriptor to serial port */
    int ser;
    ser = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (ser == -1) {
        ERR("Failed to open /dev/ttyACM0\n");
        status = EXIT_FAILURE;
        goto cleanup;
    }
    /* Setup serial port to be non-blocking, raw data, 115200 baud */
    fcntl(ser, F_SETFL, FNDELAY);
    struct termios options;
    tcgetattr(ser, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    cfmakeraw(&options);
    tcsetattr(ser, TCSANOW, &options);
    tcflush(ser, TCIOFLUSH);
    DBG("Serial port opened and configured\n");

    /* Start the access point */
    unsigned char startAP[] = {0xFF, 0x07, 0x03};
    if (write(ser, startAP, 3) < 0) {
        ERR("Failed to write startAP to serial port\n");
        status = EXIT_FAILURE;
        goto cleanup;
    }
    tcflush(ser, TCIOFLUSH);
    DBG("Access Point started\n");

    /* Data reading loop */
    int bytesRead = 0;
    int tempRead = 0;
    unsigned char data[7];
    unsigned char dataRequest[] = {0xFF, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00};
    while (!quit) {
        /* Send data request */
        DBG("Sleep for...");
        sleep(1);
        DBG(" 1 second\n");
        if (write(ser, dataRequest, 7) < 0) {
            ERR("Failed to write dataRequest to serial port \n");
            status = EXIT_FAILURE;
            goto cleanup;
        }
        DBG("dataRequest sent\n");
        /* Read 7 bytes */
        while (bytesRead < 7) {
            tempRead = read(ser, data+bytesRead, 1);
            if (tempRead != -1) {
                bytesRead += tempRead;
            }
            if (quit) {
                goto cleanup;
            }
        }
        bytesRead = 0;
        DBG("7 bytes read\n");
        /* Determine what was pressed */
        if (data[6] == 0x12 &&                  \
                 data[5] == 0x07 &&             \
                 data[4] == 0x06 &&             \
                 data[3] == 0xFF &&             \
                 data[2] == 0x00 &&             \
                 data[1] == 0x00 &&             \
                 data[0] == 0x00) {
            printf("M1 pressed\n");
        }
        else if (data[6] == 0x22 &&             \
                 data[5] == 0x07 &&             \
                 data[4] == 0x06 &&             \
                 data[3] == 0xFF &&             \
                 data[2] == 0x00 &&             \
                 data[1] == 0x00 &&             \
                 data[0] == 0x00) {
            printf("M2 pressed\n");
        }
        else if (data[6] == 0x32 &&             \
                 data[5] == 0x07 &&             \
                 data[4] == 0x06 &&             \
                 data[3] == 0xFF &&             \
                 data[2] == 0x00 &&             \
                 data[1] == 0x00 &&             \
                 data[0] == 0x00) {
            printf("S1 pressed\n");
        }
        else {
            DBG("Data values:\n\tdata[6]: %x\n\tdata[5]: %x\n\tdata[4]: %x\n\tdata[3]: %x\n\tdata[2]: %x\n\tdata[1]: %x\n\tdata[0]: %x\n", data[6], data[5], data[4], data[3], data[2], data[1], data[0]);
        }
    }

 cleanup:
    close(ser);
    DBG("Serial port closed\n");
    if (status == EXIT_FAILURE) {
        printf("Program exited with FAILURE\n");
    }
    else if (status == EXIT_SUCCESS) {
        printf("Program exited with SUCCESS\n");
    }
    exit(status);
}
