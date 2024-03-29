/*
 * Back propagation neural network training program
 * Originally written by Steven Hayes, and updated by Mengjie Zhang
 * on 25 March 1998, 8 April 1998, and 15 Aug 2007.
 *
 * First change on 25 March 1998:
 *   Plus the "training correct percentage" on the display procedure;
 *
 * Second change on 8 April 1998:
 *   -- TSS ==> mse(mean square error);
 *   -- training stop criteria:
 *          . error < ecrit (mse);
 *          . correct_percentage > percentage;
 *          . epoch > 1000000;
 * Third change on 05/09/00:
 *    -- deal with full text pattern file
 *    -- non-image input
 * Note: this program is for "on-line" network training.
 *
 * Fourth change on 15 Aug: 
 *    -- adjust the inclusion of header/library files
 */

#include <unistd.h>
#include <signal.h>
#include "backprop.h"
#include <stdlib.h>

/* local prototypes */

void display();

int done = 0;

void sigcatcher(void)
{
    printf("PID %d: Trainning Interrupted - Saving weights and terminating\n",
	    getpid());
    done = 1;
}

int main(argc, argv)
int argc;
char **argv;
{
    double error = 10000.0;
    int    e = 1;
    int    i, j;
    int errcount = 0;
    double correct_percentage = 0.0;
    double mse = 100;

    printf("Backprop Neural Network Simulator\n Version 5.0\n (C) 2000 Mengjie Zhang, VUW\n mengjie@mcs.vuw.ac.nz\n");

    epsilon = 0.2;        /* default values for parameters */
    momentum = 0.0;
    range = 0.1;
    ecrit = 0.04;

    fanin = TRUE;
    verbose = 1;
    vverbose = FALSE;

    if (argc < 3)
    {
	printf("usage: %s network patterns [weights]\n", argv[0]);
	return 0;
    }
    if (!load_network(argv [1]))
    {
        printf("Error on reading %s\n", argv [1]);
        return 0;
    }

    if (argc == 4)
    {
	printf("Loading initial weights from %s\n", argv[3]);
	if (!load_weights(argv[3]))
	{
	    printf("Error on reading %s\n", argv[3]);
	    return 0;
        }
    } 
    else 
    {
        reset_network();
        scale_weights();
    }

    if (!load_patterns(argv [2]))
    {
        printf("Error on reading %s\n", argv [2]);
        return 0;
    }

    signal(SIGINT, sigcatcher);

    while (!done)
    {
        error =    0.0;
        errcount = 0;
        for (i = 0; i < ntrain; ++i)
        {
            int oc = 0;
            int rc = 0;
            int k = 0;
            double ac = 0.0;

            compute_output(i);
            error += compute_error(i);

            /* new error calculation method here */
            for (j = nunits - units_in_layer [nlayers - 1]; j < nunits; ++j)
            {
                if (activate [j] > ac) 
                {
                    ac = activate [j];
                    oc = k;
                }
                ++k;
            }

            for (j = 0; j < units_in_layer [nlayers - 1]; ++j)
            {
                if ((1.0 - targets[i][j]) < 0.1)
                    rc = j;
            }
            if (oc != rc) {
               ++errcount;
            }
            /* new error calculation method ends here */

            compute_error_derivative(i);
            change_weights();
        }
        correct_percentage = (double)(ntrain - errcount) * 100.0 / (double)ntrain;
        mse = error / (ntrain * units_in_layer[nlayers - 1]);

        if (verbose > 0 && !(e % verbose))
        {
            printf("epoch = %d, mse = %2.3lf, Percentage = %d/%d = %2.2f%c\n", e++, mse, ntrain - errcount, ntrain, correct_percentage, '%');
            if (vverbose)
            {
                display();
            }
        }
	if (mse < ecrit)                          done = 1;
        if (correct_percentage >= corrpercent)    done = 1;
        if (e > 1000000)                          done = 1;
    }
    #ifdef DISPLAY
    display();
    #endif
    save_weights("weights.dat");
    printf("ending\n");
}

void display()
{
    int i, j;

    if (ntrain > 0)
        printf("Results of learning training data:\n");
    for (i = 0; i < ntrain; ++i)                     /* show results */
    {
        compute_output(i);

        printf("\nout:");
        for (j = nunits - units_in_layer [nlayers - 1]; j < nunits; ++j)
        {
            printf(" %2.1lf", activate [j]);
        }
        printf("\n");
    }

    if (ntest > 0)
        printf("Results of learning test data:\n");
    for (i = ntrain; i < ntrain + ntest; ++i)
    {
        compute_output(i);
        printf("\nout:");
        for (j = nunits - units_in_layer [nlayers - 1]; j < nunits; ++j)
        {
            printf(" %2.1lf", activate [j]);
        }
        printf("\n");
    }
}
