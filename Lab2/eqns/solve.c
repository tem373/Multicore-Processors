#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/*** Skeleton for Lab 2 ***/

/***** Globals ******/
float **a; /* The coefficients */
float *x;  /* The unknowns */
float *b;  /* The constants */
float err; /* The absolute relative error */
int num = 0;  /* number of unknowns */


/****** Function declarations */
void check_matrix(); /* Check whether the matrix will converge */
void get_input();  /* Read input from file */

/********************************/



/* Function definitions: functions are ordered alphabetically ****/
/*****************************************************************/

/* 
   Conditions for convergence (diagonal dominance):
   1. diagonal element >= sum of all other elements of the row
   2. At least one diagonal element > sum of all other elements of the row
 */
void check_matrix() {
    int bigger = 0; /* Set to 1 if at least one diag element > sum  */
    int i, j;
    float sum = 0;
    float aii = 0;

    for(i = 0; i < num; i++) {
        sum = 0;
        aii = fabs(a[i][i]);

        for(j = 0; j < num; j++) {
            if (j != i)
                sum += fabs(a[i][j]);
            if (aii < sum) {
                printf("The matrix will not converge.\n");
                exit(1);
            }

            if (aii > sum) {
                bigger++;
            }
        }
    }
    if(!bigger) {
        printf("The matrix will not converge\n");
        exit(1);
    }
}


/******************************************************/
/* Read input from file */
/* After this function returns:
 * a[][] will be filled with coefficients and you can access them using a[i][j] for element (i,j)
 * x[] will contain the initial values of x
 * b[] will contain the constants (i.e. the right-hand-side of the equations
 * num will have number of variables
 * err will have the absolute error that you need to reach
 */
void get_input(char filename[]) {
    FILE * fp;
    int i,j;

    fp = fopen(filename, "r");
    if(!fp) {
        printf("Cannot open file %s\n", filename);
        exit(1);
    }

    fscanf(fp,"%d ",&num);
    fscanf(fp,"%f ",&err);

    /* Now, time to allocate the matrices and vectors */
    a = (float**)malloc(num * sizeof(float*));
    if(!a) {
        printf("Cannot allocate a!\n");
        exit(1);
    }

    for(i = 0; i < num; i++) {
        a[i] = (float *)malloc(num * sizeof(float));
        if(!a[i]) {
            printf("Cannot allocate a[%d]!\n",i);
            exit(1);
        }
    }

    x = (float *) malloc(num * sizeof(float));
    if(!x) {
        printf("Cannot allocate x!\n");
        exit(1);
    }

    b = (float *) malloc(num * sizeof(float));
    if(!b) {
        printf("Cannot allocate b!\n");
        exit(1);
    }

    /* Now .. Filling the blanks */
    /* The initial values of Xs */
    for(i = 0; i < num; i++) {
        fscanf(fp, "%f ", &x[i]);
    }
    for(i = 0; i < num; i++) {
        for(j = 0; j < num; j++) {
            fscanf(fp, "%f ", &a[i][j]);
        }
        /* reading the b element */
        fscanf(fp,"%f ",&b[i]);
    }
 
    fclose(fp);

}

int updateUnknowns(int n_iter) {
    /* MPI setup */
    int comm_sz;
    int ps_id;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &ps_id);

    // Calculate the size of the "batch"
    int batch_sz = (int) (num / comm_sz);
    int batch_start, batch_end;
    int offset = num % comm_sz;
    int all_done = 1;
    int proc_done = 1;
    float *x_old = (float *) malloc(num * sizeof(float));
    float *x_new  = (float *) malloc(num * sizeof(float));

    // Update value element wise
    if (ps_id == 0) { /* Master process */
//        MPI_Bcast(x, num, MPI_FLOAT, 0, MPI_COMM_WORLD);
//        MPI_Bcast(a, num, MPI_FLOAT, 0, MPI_COMM_WORLD);
//        MPI_Bcast(b, num, MPI_FLOAT, 0, MPI_COMM_WORLD);

        batch_start = 0;
        batch_end = batch_sz-1;

        x_old[ps_id] = x[ps_id]; /* move x_old = x */
        for (int i = 1; i < comm_sz; i++) {
            MPI_Send(&x_old[0], num, MPI_FLOAT, i, 1, MPI_COMM_WORLD); /* send chunks to children */
            MPI_Send(&x_new[i * batch_sz], batch_sz, MPI_FLOAT, i, 1, MPI_COMM_WORLD);
        }

        for (int i = batch_start; i < batch_end + offset; i++) { /* do work */
            float x_prev = x[i];
            float x_next = b[i]; // initialize to coefficient
            for (int j = 0; j < num; j++) {
                // Use the equation to update the values of
                if (i != j) {
                    x_next -= a[i][j] * x_old[j];
                }
            }
            x_next /= a[i][i];

            // Check error TODO: MPI reduce this?
            float rel_err = fabs((x_next - x_prev) / x_next);
            if (rel_err >= err) {
                proc_done = 0;
            }

            x_new[i] = x_next; /* x_next -> x */
        }
        all_done = proc_done;

        for (int i = 1; i < comm_sz; i++) {
            MPI_Recv(&proc_done, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /* receive child work */
            MPI_Recv(&x_new[i * batch_sz], batch_sz, MPI_FLOAT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /* receive child work */
            all_done *= proc_done;
        }
        //x_new[i] = x_new; /* x_next -> x */
        if (all_done == 0) {
            MPI_Send(&all_done, 1, MPI_INT, 0, 1, MPI_COMM_WORLD); /* send halt to children */
        }

    }
    else if (ps_id != 0) { /* All other processes */
        batch_start = ps_id * batch_sz;
        batch_end = ((ps_id+1) * batch_sz)-1;

        MPI_Recv(&x_old[0], num, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /* receive chunk */
        MPI_Recv(&x_new[batch_start], batch_sz, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /* receive chunk */

        for (int i = batch_start; i < batch_end; i++) { /* do work */
            float x_prev = x[i];
            float x_next = b[i]; // initialize to coefficient
            for (int j = 0; j < num; j++) {
                // Use the equation to update the values of
                if (i != j) {
                    x_next -= a[i][j] * x_old[j];
                }
            }
            x_next /= a[i][i];

            // Check error
            // TODO: MPI reduce this?
            float rel_err = fabs((x_next - x_prev) / x_next);
            if (rel_err >= err) {
                proc_done = 0;
            }

            x_new[i] = x_next;
        }

        MPI_Send(&proc_done, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        MPI_Send(&x_new[batch_start], batch_sz, MPI_FLOAT, 0, 1, MPI_COMM_WORLD); /* send chunks */
        MPI_Recv(&all_done, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /* receive halt */

        if (all_done) {
            return all_done;
        }
    }

    for (int i=0; i < num; i++) { /* set the new array to current x values for next round */
        x[i] = x_new[i];
    }

    return all_done;
}

/************************************************************/


int main(int argc, char *argv[]) {

    int i;
    int nit = 0; /* number of iterations */
    FILE * fp;
    char output[100] ="";
  
    if( argc != 2) {
        printf("Usage: ./solve filename\n");
        exit(1);
    }
  
    /* Read the input file and fill the global data structure above */
    get_input(argv[1]);
 
    /* Check for convergence condition */
    /* This function will exit the program if the coefficient will never converge to
    * the needed absolute error.
    * This is not expected to happen for this programming assignment.
    */
    check_matrix();

    MPI_Init(NULL, NULL);

    // Iterations loop
    int foundSol = 0;
    while (!foundSol) {
        foundSol = 1;
        foundSol = updateUnknowns(nit);
        nit++;
    }

    MPI_Finalize();

    /* Writing results to file */
    sprintf(output,"my%d.sol",num);
    fp = fopen(output,"w");
    if(!fp) {
        printf("Cannot create the file %s\n", output);
        exit(1);
    }
    
    for( i = 0; i < num; i++) {
        fprintf(fp, "%f\n", x[i]);
    }

    printf("Unknowns\n");
    for( i = 0; i < num; i++) {
        printf("%f ", x[i]);
    }
    printf("\n");

    printf("total number of iterations: %d\n", nit);
 
    fclose(fp);
    exit(0);

}
