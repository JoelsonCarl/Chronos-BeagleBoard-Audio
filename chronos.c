/* Standard Linux Headers */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

/* Local includes */
#include "debug.h"
#include "chronos.h"

/*********************************************
 * Define commands for access point
 *
 *  Background Polling
 *      PC to AP: FF 20 07 00 00 00 00
 *      Response: FF 06 07 xx xx xx xx
 *      xx xx xx xx: 4 byte watch address
 *
 *  Start Access Point
 *      PC to AP: FF 07 03
 *      Response: FF 06 03
 *
 *  Check AP Command (functionality not known)
 *      PC to AP: FF 00 04 00
 *      Response: FF 06 04 03
 *      It may be returning the state of the watch or AP
 *
 *  Request Data
 *      PC to AP: FF 08 07 00 00 00 00
 *      Response: FF 06 07 tt xx yy zz
 *          tt: data type (FF: no data, 01: valid acc data)
 *              -12 (M1 pressed)(xx,yy,zz = 00)
 *              -22 (M2 pressed)(xx,yy,zz = 00)
 *              -32 (S1 pressed)(xx,yy,zz = 00)
 *              -01 (valid acc data)(xx,yy,zz = acc data)
 *              -FF (no/invalid data)
 *
 *  Stop Access Point
 *      PC to AP: FF 07 03 FF 09 03
 *      AP to PC: FF 06 03
 *      ("Start" command followed by FF 09 03)
 **********************************************/
//unsigned char CMD_BackgroundPoll[] = {0xFF, 0x20, 0x07, 0x00, 0x00, 0x00, 0x00};
//unsigned char CMD_BackgroundPollResp[] = {0xFF, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00};
unsigned char CMD_StartAP[] = {0xFF, 0x07, 0x03};
unsigned char CMD_StartAPResp[] = {0xFF, 0x06, 0x03};
//unsigned char CMD_CheckAP[] = {0xFF, 0x00, 0x04, 0x00};
//unsigned char CMD_CheckAPResp[] = {0xFF, 0x06, 0x04, 0x03};
unsigned char CMD_RequestData[] = {0xFF, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00};
unsigned char CMD_RequestDataResp[] = {0xFF, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00};
unsigned char CMD_StopAP[] = {/*0xFF, 0x07, 0x03,*/ 0xFF, 0x09, 0x03};
//unsigned char CMD_StopAPResp[] = {0xFF, 0x06, 0x03};

/* Global variable to register quit */
int quit = 0;


/**********************************************
 * main
 *
 * start of program
 **********************************************/
int main(int argc, char *argv[]) {
    /* Array for storing data from serial port */
    unsigned char data[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* Set the signal callback for Ctrl-C */
    signal(SIGINT, signal_handler);
    DBG("Ctrl-C handler set.\n");

    /* Open file descriptor to serial port */
    int ser;
    ser = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (ser == -1) {
        ERR("Failed to open /dev/ttyACM0\n");
        cleanup(ser, EXIT_FAILURE);
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

    /* Start the AP */
    DBG("About to write to serial start ap\n");
    writeSerial(ser, CMD_StartAP, sizeof(CMD_StartAP));
    DBG("Wrote start AP\n");
    /* Verify the response is correct */
    readSerial(ser, sizeof(CMD_StartAPResp), data);
    if (!(data[0] == CMD_StartAPResp[0] &&
          data[1] == CMD_StartAPResp[1] &&
          data[2] == CMD_StartAPResp[2])) {
        ERR("Incorrect response after starting AP\n\tdata[0] = %x\n\tdata[1] = %x\n\tdata[2] = %x\n", data[0], data[1], data[2]);
        cleanup(ser, EXIT_FAILURE);
    }
    DBG("Access Point started\n");

    /* Main program loop */
    int playing = 1;
    int effect = 0;
    int decode = 0;
    start_pipeline(effect);
    printf("Program Started.\n");
    printf("Starting Effect (0): lowpass\n");
    while (!quit) {
        sleep(1);
        /* Send data request */
        writeSerial(ser, CMD_RequestData, sizeof(CMD_RequestData));
        /* Read response */
        readSerial(ser, sizeof(CMD_RequestDataResp), data);
        /* Determine Response */
        decode = decodeData(data);
        if (decode == M1) {
            /* M1 (top left) button pressed */
            effect += 1;
            if (effect == NUM_EFFECTS) {
                effect = 0;
            }
            if (playing) {
                restart_pipeline(effect);
            }
            printf("Switched to effect %d.\n", effect);
        }
        else if (decode == M2) {
            /* M2 (bottom left) button pressed */
            printf("M2 pressed\n");
        }
        else if (decode == S1) {
            /* S1 (top right) button pressed */
            if (!playing) {
                start_pipeline(effect);
                printf("Audio Started (Effect: %d)\n", effect);
                playing = 1;
            }
            else {
                stop_pipeline();
                printf("Audio Stopped\n");
                playing = 0;
            }
        }
        else if (decode == ACC_INVALID) {
            /* Invalid Accelerometer Data */
            DBG("Invalid Accelerometer Data\n");
        }
        else if (decode == ACC_VALID) {
            /* Valid Accelerometer Data */
            DBG("Valid Accelerometer Data\n\tX = %x\n\tY = %x\n\tZ = %x\n", data[4], data[5], data[6]);
            configure_effect(effect, data);
        }
        else {
            /* Unknown */
            DBG("Unknown Response\nData values:\n\tdata[6]: %x\n\tdata[5]: %x\n\tdata[4]: %x\n\tdata[3]: %x\n\tdata[2]: %x\n\tdata[1]: %x\n\tdata[0]: %x\n", data[6], data[5], data[4], data[3], data[2], data[1], data[0]);
        }        
    }

    /* Ctrl-C pressed -> quit with success */
    cleanup(ser, EXIT_SUCCESS);
    return 0;
}


/**********************************************
 * decodeData
 *  Determines what the data from the RF access
 *  point means
 *
 *  Arguments
 *      data - the data from the access point
 *
 *  Returns
 *      an int representing what happened
 **********************************************/
int decodeData(unsigned char *data) {
    if (data[0] == CMD_RequestDataResp[0] &&
        data[1] == CMD_RequestDataResp[1] &&
        data[2] == CMD_RequestDataResp[2] &&
        data[3] == 0x12 &&
        data[4] == CMD_RequestDataResp[4] &&
        data[5] == CMD_RequestDataResp[5] &&
        data[6] == CMD_RequestDataResp[6]) {
        return M1;
    }
    else if (data[0] == CMD_RequestDataResp[0] &&
             data[1] == CMD_RequestDataResp[1] &&
             data[2] == CMD_RequestDataResp[2] &&
             data[3] == 0x22 &&
             data[4] == CMD_RequestDataResp[4] &&
             data[5] == CMD_RequestDataResp[5] &&
             data[6] == CMD_RequestDataResp[6]) {
        return M2;
    }
    else if (data[0] == CMD_RequestDataResp[0] &&
             data[1] == CMD_RequestDataResp[1] &&
             data[2] == CMD_RequestDataResp[2] &&
             data[3] == 0x32 &&
             data[4] == CMD_RequestDataResp[4] &&
             data[5] == CMD_RequestDataResp[5] &&
             data[6] == CMD_RequestDataResp[6]) {
        return S1;
    }
    else if (data[0] == CMD_RequestDataResp[0] &&
             data[1] == CMD_RequestDataResp[1] &&
             data[2] == CMD_RequestDataResp[2] &&
             data[3] == 0xFF) {
        return ACC_INVALID;
    }
    else if (data[0] == CMD_RequestDataResp[0] &&
             data[1] == CMD_RequestDataResp[1] &&
             data[2] == CMD_RequestDataResp[2] &&
             data[3] == 0x01) {
        return ACC_VALID;
    }
    else {
        return UNKNOWN;
    }
}


/**********************************************
 * configure_effect
 *  Changes the current effect using the
 *  accelerometer data
 *
 *  Arguments
 *      effect - the current effect
 *      data - the data from the access point
 *
 *  Returns
 *      void
 **********************************************/
void configure_effect(int effect, unsigned char *data) {
    if (effect == LPF) {
        // Low-pass Filter
        // X controls cutoff frequency
        int x = data[4];
        float cutoff = 1500;
        if (x >= 0x80 && x <= 0xDA) {
            cutoff = 0;
        }
        else if (x >= 0xDB && x <= 0xE4) {
            cutoff = 350;
        }
        else if (x >= 0xE5 && x <= 0xEE) {
            cutoff = 700;
        }
        else if (x >= 0xEF && x <= 0xF8) {
            cutoff = 1050;
        }
        else if (x >= 0xF9 || x <= 0x02) {
            cutoff = 1400;
        }
        else if (x >= 0x03 && x <= 0x0D) {
            cutoff = 1750;
        }
        else if (x >= 0x0E && x <= 0x17) {
            cutoff = 2100;
        }
        else if (x >= 0x18 && x <= 0x21) {
            cutoff = 2450;
        }
        else if (x >= 0x22 && x <= 0x2B) {
            cutoff = 2800;
        }
        else if (x >= 0x2C && x <= 0x7F) {
            cutoff = 3150;
        }        
        else {
            cutoff = 1500;
        }
        DBG("LPF Configure\n\tcutoff = %f\n", cutoff);
        configure_LPF(cutoff);
    }
    else if (effect == BPF) {
        // Band-pass Filter
        // X controls center frequency
        // Y controls bandwidth
        int x = data[4];
        int y = data[5];
        float center = 1500;
        float bandwidth = 500;
        if (x >= 0x80 && x <= 0xDA) {
            center = 0;
        }
        else if (x >= 0xDB && x <= 0xE4) {
            center = 350;
        }
        else if (x >= 0xE5 && x <= 0xEE) {
            center = 700;
        }
        else if (x >= 0xEF && x <= 0xF8) {
            center = 1050;
        }
        else if (x >= 0xF9 || x <= 0x02) {
            center = 1400;
        }
        else if (x >= 0x03 && x <= 0x0D) {
            center = 1750;
        }
        else if (x >= 0x0E && x <= 0x17) {
            center = 2100;
        }
        else if (x >= 0x18 && x <= 0x21) {
            center = 2450;
        }
        else if (x >= 0x22 && x <= 0x2B) {
            center = 2800;
        }
        else if (x >= 0x2C && x <= 0x7F) {
            center = 3150;
        }        
        else {
            center = 1500;
        }
        if (y >= 0x80 && y <= 0xDA) {
            bandwidth = 100;
        }
        else if (y >= 0xDB && y <= 0xE4) {
            bandwidth = 300;
        }
        else if (y >= 0xE5 && y <= 0xEE) {
            bandwidth = 500;
        }
        else if (y >= 0xEF && y <= 0xF8) {
            bandwidth= 700;
        }
        else if (y >= 0xF9 || y <= 0x02) {
            bandwidth= 900;
        }
        else if (y >= 0x03 && y <= 0x0D) {
            bandwidth= 1100;
        }
        else if (y >= 0x0E && y <= 0x17) {
            bandwidth= 1300;
        }
        else if (y >= 0x18 && y <= 0x21) {
            bandwidth= 1500;
        }
        else if (y >= 0x22 && y <= 0x2B) {
            bandwidth= 1700;
        }
        else if (y >= 0x2C && y <= 0x7F) {
            bandwidth= 1900;
        }        
        else {
            bandwidth = 500;
        }
        DBG("BPF Configure\n\tcenter = %f\n\tbandwidth = %f\n", center, bandwidth);
        configure_BPF(center, bandwidth);
    }
}


/**********************************************
 * cleanup
 *  Cleans up and exits the program
 *
 *  Arguments
 *      ser - file descriptor of serial port
 *      status - program exit status
 *
 *  Returns
 *      exits the program
 **********************************************/
void cleanup(int ser, int status) {
    /* Stop the AP */
    if (write(ser, CMD_StopAP, sizeof(CMD_StopAP)) < 0) {
        ERR("Failed to write CMD_StopAP to serial port\n");
        status = EXIT_FAILURE;
    }
    DBG("AP Stopped\n");
    /* Close the serial port */
    close(ser);
    DBG("Serial port closed\n");
    /* Exit the program */
    if (status == EXIT_FAILURE) {
        printf("Program exited with FAILURE\n");
    }
    else if (status == EXIT_SUCCESS) {
        printf("Program exited with SUCCESS\n");
    }
    exit(status);
}


/**********************************************
 * writeSerial
 *  Writes data to the serial port
 *
 *  Arguments
 *      ser - file descriptor of serial port
 *      data - data to write
 *      len - length of data to write
 *
 *  Returns
 *      void
 **********************************************/
void writeSerial(int ser, unsigned char *data, int len) {
    if (write(ser, data, len) < 0) {
        ERR("Failed to write data to serial port\n");
        cleanup(ser, EXIT_FAILURE);
    }
}


/**********************************************
 * readSerial
 *  Reads from the serial port
 *
 *  Arguments
 *      ser - file descriptor of serial port
 *      len - length of data to read
 *      data - location to store read data
 *
 *  Returns
 *      void
 **********************************************/
void readSerial(int ser, int len, unsigned char *data) {
    int bytesRead = 0;
    int tempRead = 0;
    while (bytesRead < len) {
        tempRead = read(ser, data+bytesRead, len-bytesRead);
        if (tempRead != -1) {
            bytesRead += tempRead;
        }
        if (quit) {
            cleanup(ser, EXIT_SUCCESS);
        }
    }
}


/**********************************************
 * signal_handler
 *
 * Code to execute upon pressing Ctrl-C
 **********************************************/
/**********************************************
 * signal_handler
 *  Code that executes upon pressing Ctrl-C
 *
 *  Arguments
 *      signal - the interrupting signal
 *
 *  Returns
 *      void
 **********************************************/
void signal_handler(int signal) {
    DBG("Ctrl-C pressed, cleaning up and exiting...\n");
    quit = 1;
}
