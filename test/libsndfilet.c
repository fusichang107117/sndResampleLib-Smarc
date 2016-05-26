/*
 *  gcc libsndfilet.c -lsndfile
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sndfile.h>

#define BLOCK_SIZE 512

    static void
print_usage (char *progname)
{   printf ("\nUsage : %s <input file> <output file>\n", progname) ;
    puts ("\n"
            "    Where the output file will contain a line for each frame\n"
            "    and a column for each channel.\n"
         ) ;

} /* print_usage */

    static void
convert_to_text (SNDFILE * infile, FILE * outfile, int channels)
{   float buf [channels * BLOCK_SIZE] ;
    int k, m, readcount ;

    while ((readcount = sf_readf_float (infile, buf, BLOCK_SIZE)) > 0)
    {   for (k = 0 ; k < readcount ; k++)
        {   for (m = 0 ; m < channels ; m++)
            fprintf (outfile, " % 12.10f", buf [k * channels + m]) ;
            fprintf (outfile, "\n") ;
        } ;
    } ;

    return ;
} /* convert_to_text */

    int
main (int argc, char * argv [])
{   char        *progname, *infilename, *outfilename ;
    SNDFILE     *infile = NULL ;
    FILE        *outfile = NULL ;
    SF_INFO     sfinfo ;

    progname = strrchr (argv [0], '/') ;
    progname = progname ? progname + 1 : argv [0] ;

    if (argc != 3)
    {   print_usage (progname) ;
        return 1 ;
    } ;

    infilename = argv [1] ;
    outfilename = argv [2] ;

    if (strcmp (infilename, outfilename) == 0)
    {   printf ("Error : Input and output filenames are the same.\n\n") ;
        print_usage (progname) ;
        return 1 ;
    } ;

    if (infilename [0] == '-')
    {   printf ("Error : Input filename (%s) looks like an option.\n\n", infilename) ;
        print_usage (progname) ;
        return 1 ;
    } ;

    if (outfilename [0] == '-')
    {   printf ("Error : Output filename (%s) looks like an option.\n\n", outfilename) ;
        print_usage (progname) ;
        return 1 ;
    } ;

    memset (&sfinfo, 0, sizeof (sfinfo)) ;

    sfinfo.samplerate = 44100;
    sfinfo.format = SF_FORMAT_RAW | SF_FORMAT_PCM_16;
    sfinfo.channels = 1;
    sfinfo.frames = 0;

    if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
    {   printf ("Not able to open input file %s.\n", infilename) ;
        puts (sf_strerror (NULL)) ;
        return 1 ;
    } ;

    /* Open the output file. */
    if ((outfile = fopen (outfilename, "w")) == NULL)
    {   printf ("Not able to open output file %s : %s\n", outfilename, sf_strerror (NULL)) ;
        return 1 ;
    } ;

    fprintf (outfile, "# Converted from file %s.\n", infilename) ;
    fprintf (outfile, "# Channels %d, Sample rate %d\n", sfinfo.channels, sfinfo.samplerate) ;

    convert_to_text (infile, outfile, sfinfo.channels) ;

    sf_close (infile) ;
    fclose (outfile) ;

    return 0 ;
} /* main */

