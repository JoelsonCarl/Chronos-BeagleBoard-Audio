/* Standard Linux Headers */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

/* Local includes */
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
    writeSerial(ser, CMD_StartAP, sizeof(CMD_StartAP));
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
    int sysRet = 0;
    int playing = 0;
    int mode = 0;
    int decode = 0;
    unsigned char acc[] = {0x00, 0x00, 0x00}; // {X, Y, Z}
    printf("Program Started.\n");
    printf("Starting Mode (0): Normal Audio Thru\n");
    while (!quit) {
        DBG("Sleep for...");
        sleep(1);
        DBG(" 1 second\n");
        /* Send data request */
        writeSerial(ser, CMD_RequestData, sizeof(CMD_RequestData));
        /* Read response */
        readSerial(ser, sizeof(CMD_RequestDataResp), data);
        /* Determine Response */
        decode = decodeData(data);
        if (decode == M1) {
            /* M1 (top left) button pressed */
            mode += 1;
            if (mode == NUM_MODES) {
                mode = 0;
            }
            if (playing) {
                sysRet = killAudio();
                if (sysRet == -1) {
                    ERR("Failed to stop audio while switching modes.\n\tmode = %d\n", mode);
                    cleanup(ser, EXIT_FAILURE);
                }
                sysRet = playAudio(mode);
                if (sysRet == -1) {
                    ERR("Failed to start audio (mode switch)\n\tmode = %d\n", mode);
                    cleanup(ser, EXIT_FAILURE);
                }

            }
            printf("Switched to mode %d.\n", mode);
        }
        else if (decode == M2) {
            /* M2 (bottom left) button pressed */
            printf("M2 pressed\n");
        }
        else if (decode == S1) {
            /* S1 (top right) button pressed */
            if (!playing) {
                sysRet = playAudio(mode);
                if (sysRet == -1) {
                    ERR("Failed to Start Audio\n\tmode = %d\n", mode);
                    cleanup(ser, EXIT_FAILURE);
                }
                printf("Audio Started\n");
                playing = 1;
            }
            else {
                sysRet = killAudio();
                if (sysRet == -1) {
                    ERR("Failed to Stop Audio\n\tmode = %d\n", mode);
                    cleanup(ser, EXIT_FAILURE);
                }
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
            acc[0] = data[4];
            acc[1] = data[5];
            acc[2] = data[6];
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
 * playAudio
 *  Launches the GStreamer pipeline
 *
 *  Arguments
 *      mode - selects GStreamer pipeline mode
 *
 *  Returns
 *      the result of the 'system' call
 **********************************************/
int playAudio(int mode) {
    int systemRet = 0;
    switch (mode) {
    case 0:
        systemRet = system("gst-launch alsasrc ! alsasink > /dev/null &");
        break;
    case 1:
        systemRet = system("gst-launch alsasrc ! audioconvert ! audiocheblimit mode=low-pass cutoff=1000 ! audioconvert ! alsasink > /dev/null &");
        break;
    case 2:
        systemRet = system("gst-launch alsasrc ! audioconvert ! audioecho delay=500000000 intensity=0.6 feedback=0.4 ! audioconvert ! alsasink > /dev/null &");
        break;
    default:
        systemRet = system("gst-launch alsasrc ! alsasink > /dev/null &");
        break;
    }
    return systemRet;
}


/**********************************************
 * killAudio
 *  Kills the GStreamer pipeline by sending it
 *  an interrupt signal
 *
 *  Arguments
 *      none
 *
 *  Returns
 *      the result of the 'system' call
 **********************************************/
int killAudio(void) {
    return system("kill -2 `ps -ef | grep gst-launch-0.10 | head -n 1 | awk '{print $2}'`");
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
