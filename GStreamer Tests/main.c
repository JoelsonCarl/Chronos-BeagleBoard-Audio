#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "pipe.h"

#define CUTOFF_MAX	500
#define CUTOFF_STEP	100

int main( int argc, const char* argv[] )
{
	float cutoff = 0;
	pthread_t pipeline_id;
	
	printf("[main]:\tCreating run_pipeline thread\n");
	fflush(stdout);
	start_pipeline(0);
	while(1){
		if(cutoff == CUTOFF_MAX)
			break;
			//cutoff = 0;
		else
			cutoff = cutoff + CUTOFF_STEP;
		printf("[main]:\tSetting Cutoff Frequency to %f\n", cutoff);
		fflush(stdout);
		configure_LPF(cutoff);
		sleep(1);
	}
	printf("[main]:\tChanging Effect to BPF\n");
	restart_pipeline(1);
	configure_BPF(2000, 4000);
	while(1){
		sleep(1);
	}
}
