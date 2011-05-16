#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "pipe.h"

#define CUTOFF_MAX	500
#define CUTOFF_STEP	100

int main( int argc, const char* argv[] )
{
	float cutoff = 0;
	
	start_pipeline(LPF);
	while(1){
		if(cutoff == CUTOFF_MAX)
			break;
		else
			cutoff = cutoff + CUTOFF_STEP;
		configure_LPF(cutoff);
		sleep(1);
	}
	restart_pipeline(BPF);
	configure_BPF(2000, 1000);
	while(1){
		sleep(1);
	}
}
