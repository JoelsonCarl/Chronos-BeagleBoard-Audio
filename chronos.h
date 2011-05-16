/* Defines */
#define NUM_EFFECTS 2
#define LPF         0
#define BPF         1
//Return values for decodeData()
#define M1          0
#define M2          1
#define S1          2
#define ACC_VALID   3
#define ACC_INVALID 4
#define UNKNOWN     5

/* Function prototypes */
int decodeData(unsigned char *data);
void configure_effect(int effect, unsigned char *data);
void cleanup(int ser, int status);
void writeSerial(int ser, unsigned char *data, int len);
void readSerial(int ser, int len, unsigned char *data);
void signal_handler(int signal);

/* Pipe Functions */
extern void start_pipeline(int effect_num);
extern void stop_pipeline();
extern void restart_pipeline(int effect_num);
extern void configure_LPF(float cutoff);
extern void configure_BPF(float center, float bandwidth);
extern void set_effect(int effect_num);
