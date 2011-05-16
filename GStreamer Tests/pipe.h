#define LPF	0
#define BPF 1

void start_pipeline(int effect_num);
void stop_pipeline();
void restart_pipeline(int effect_num);
void configure_LPF(float cutoff);
void configure_BPF(float center, float bandwidth);
void set_effect(int effect_num);
