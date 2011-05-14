/*
 *   audio_process.c
 */

//* Standard Linux headers **
#include     <stdio.h>                          // Always include stdio.h
#include     <stdlib.h>                         // Always include stdlib.h
#include     <string.h>                         // Defines memcpy

//* Application headers **
#include     "debug.h"                          // DBG and ERR macros
#include     "audio_process.h"			// 

#include     <dsplib64plus.h>
#include	 <DSP_fir_gen.h>
#include	 <DSP_fir_gen_cn.h>

#define 	nearestFour(x)	(x/4)*4

// Here's where we processing the audio
// Format is left and right interleaved.

int audio_process(short *outputBuffer, short *inputBuffer, int samples) {
int i;
short *leftBuffer_in,  *rightBuffer_in;
short *leftBuffer_out, *rightBuffer_out;

short *h;
int nh, nr;


    DBG("Allocating buffers for left and right channels\n");
    // memcpy((char *)outputBuffer, (char *)inputBuffer, 2*samples);
    leftBuffer_in = (short*)malloc(samples*sizeof(short));
    rightBuffer_in = (short*)malloc(samples*sizeof(short));

    leftBuffer_out = (short*)malloc(samples*sizeof(short));
    rightBuffer_out = (short*)malloc(samples*sizeof(short));
	
    DBG("Distributing Samples in buffers\n");
    for(i=0; i<samples; i++) {
		leftBuffer_in[i]  = inputBuffer[2*i];
		rightBuffer_in[i] = inputBuffer[2*i+1];
    }

	// PROCESSING GOES HERE
	DBG("Filtering...");
	nh = 5;

	h = (short*)malloc(nh*sizeof(short));
	h[0]=0.2;
	h[1]=0.2;
	h[2]=0.2;
	h[3]=0.2;
	h[4]=0.2;

	nr = nearestFour(samples-nh+1);

	DSP_fir_gen(leftBuffer_in, h, leftBuffer_out, nh, nr);
	DSP_fir_gen(rightBuffer_in, h, rightBuffer_out, nh, nr);

	DBG("Padding output buffers");
	for(i=nr; i<samples; i++) {
		leftBuffer_out=leftBuffer_in;
		rightBuffer_out=rightBuffer_in;
	}

    DBG("Interleaving samples into output buffer\n");
    for(i=0; i<samples; i++) {
		outputBuffer[2*i] = leftBuffer_out[i];
		outputBuffer[2*i+1] = rightBuffer_out[i];
    }

    return 0;
}
