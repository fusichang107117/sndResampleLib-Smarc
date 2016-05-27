/*
 *  gcc *.o resample.c -lsndfile -lm
 * */
#include "../smarc.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 8192

int main(int argc, char** argv)
{
    int fsin = 44100; // input samplerate
    int fsout = 16000; // output samplerate
    double bandwidth = 0.95;  // bandwidth
    double rp = 0.1; // passband ripple factor
    double rs = 140; // stopband attenuation
    double tol = 0.000001; // tolerance

    // initialize smarc filter
    struct PFilter* pfilt = smarc_init_pfilter(fsin, fsout, bandwidth, rp, rs, tol, NULL, 0);
    if (pfilt == NULL)
        goto exit;

    // initialize smarc filter state
    struct PState* pstate = smarc_init_pstate(pfilt);

    // initialize buffers
    const int IN_BUF_SIZE = BUF_SIZE;
    const int OUT_BUF_SIZE = (int) smarc_get_output_buffer_size(pfilt,IN_BUF_SIZE);
    double* inbuf = malloc(IN_BUF_SIZE * sizeof(double));
    double* outbuf = malloc(OUT_BUF_SIZE * sizeof(double));

    uint16_t* inbuft = malloc(IN_BUF_SIZE * sizeof(uint16_t));
    uint16_t* outbuft = malloc(OUT_BUF_SIZE * sizeof(uint16_t));

    int readed = 0;
    int written = 0;

    int infd ;
    infd = open("./2.pcm", O_RDWR);
    int outfd;
    outfd = open("./out.pcm", O_RDWR | O_CREAT, 0644);

    // resample audio
    int i =0;
    int j =0;
    int w =0;
    while (1) {

        // read input signal block into inbuf
        //read = read_my_input_signal(inbuf,BUF_SIZE);
        readed = read(infd, inbuft, IN_BUF_SIZE*sizeof(uint16_t));
        if (readed == 0) {
            // reached end of file, have to flush last values
            printf("read EOF\n");
            break;
        }
        
        for (i=0; i < BUF_SIZE; ++i)
        {
           printf("%d %#x ", inbuft[i], inbuft[i]);
           inbuf[i] = (double)inbuft[i];
           printf("%f \n", inbuf[i]);
        }
        // resample signal block
        written = smarc_resample(pfilt, pstate, inbuf, IN_BUF_SIZE, outbuf, OUT_BUF_SIZE);

        // do what you want with your output
        //write_my_resampled_signal(outbuf, written);
        for (j=0; j < written; ++j)
        {
           printf("%f ", outbuf[i]);
           outbuft[j] = (uint16_t)outbuf[j];
           printf("%d %#x\n", outbuft[i], outbuft[i]);
        }
        
        write(outfd, outbuft, written*sizeof(uint16_t));
    }

    // flushing last values
    while (1) {
        written = smarc_resample_flush(pfilt, pstate, outbuf,
                OUT_BUF_SIZE);

        // do what you want with your output
        //write_my_resampled_signal(outbuf, written);
        for (j=0 ; j < written; ++j)
           outbuft[j] = (uint16_t)outbuf[j];
        write(outfd, outbuft, written*sizeof(uint16_t));

        // if written<OUT_BUF_SIZE then there will be no more output
        if (written<OUT_BUF_SIZE)
            break;
    }

    // you are done with converting your signal.
    // If you want to reuse the same converter to process another signal
    // just reset the state:
    //
    // smarc_reset_pstate(pstate,pfilt);
    //

exit:
    close(infd);
    close(outfd);

    // release buffers
    free(inbuf);
    free(outbuf);

    // release smarc filter state
    smarc_destroy_pstate(pstate);

    // release smarc filter
    smarc_destroy_pfilter(pfilt);

    return 0;
}
