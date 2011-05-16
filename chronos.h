/* Debugging and Error Output */
#ifdef _DEBUG
    #define DBG(fmt, args...) fprintf(stderr, "Debug: " fmt, ## args)
#else
    #define DBG(fmt, args...)
#endif

#define ERR(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)

/* Defines */
#define NUM_MODES 3
//Return values for decodeData()
#define M1          0
#define M2          1
#define S1          2
#define ACC_VALID   3
#define ACC_INVALID 4
#define UNKNOWN     5

/* Function prototypes */
void cleanup(int ser, int status);
void writeSerial(int ser, unsigned char *data, int len);
void readSerial(int ser, int len, unsigned char *data);
int decodeData(unsigned char *data);
int playAudio(int mode);
int killAudio(void);
void signal_handler(int signal);
