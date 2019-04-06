#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define MAX_SIZE 10

// Global variables
int cost = 0;
int num_cities = 0;
int num_threads = 0;
int cost_matrix[MAX_SIZE][MAX_SIZE];    // max 10 cities
int visited_cities[MAX_SIZE];           // max 10 cities

/********************************** HELPER FUNCTIONS ********************************/
int searchroute(int c) {
    int count, nearest_city = 999;
    int minimum = 999, temp;
    
    #pragma omp parallel num_threads(num_threads) private(count)
    for(count = 0; count < num_cities; count++) {
        if((cost_matrix[c][count] != 0) && (visited_cities[count] == 0)) {
            if(cost_matrix[c][count] + cost_matrix[count][c] < minimum) {
                minimum = cost_matrix[count][0] + cost_matrix[c][count];
                temp = cost_matrix[c][count];
                nearest_city = count;
            }
        }
    }
    if(minimum != 999) {
        cost = cost + temp;
    }
    return nearest_city;
}

void minimum_cost(int city) {
    // Follows the branch and bound algorithm - calculates minimum cost per city
    // from undirected graph in searchroute() method and assigns each city a new next path
    int nearest_city;
    visited_cities[city] = 1;
    printf("%d ", city);
    nearest_city = searchroute(city);

    // Base case - do nothing here to reflect "no return trip"
    if(nearest_city == 999) {
        return;
    }
    minimum_cost(nearest_city);
}

/*********************************** MAIN FUNCTION **********************************/
int main(int argc, char* argv[]) {
    // Time measurements
    clock_t start, end;
    start = clock();
    
    // Parse command-line arguments and check for correct usage
    if(argc != 4) {
        printf("Incorrect number of arguments provided - please provide arguments as detailed below:\n");
        printf("./ptsm x t inputfile.txt\n");
        exit(1);
    }

    // Initialize variables + assign input numbers
    num_cities = atoi(argv[1]);
    num_threads = atoi(argv[2]);

    // Open File
    FILE *infile = fopen(argv[3], "r");
    if(infile == NULL) {
        fprintf(stderr, "Unable to open input file.\n");
        exit(1);
    }
    // Read in cost matrix
    for(int row = 0; row < num_cities; row++) {
        for(int col = 0; col < num_cities; col++) {
            int num;
            fscanf(infile, "%d ", &num);
            cost_matrix[row][col] = num;
        }
    }
    fclose(infile);

    // Output results
    printf("Optimal path: ");
    minimum_cost(0);  // Start at city '0' by convention
    printf("\n");
    printf("Distance: %d\n", cost);

    end = clock();
    double time_taken = ((double)end - (double)start)/CLOCKS_PER_SEC; // in seconds
    printf("Program took %.20f seconds to execute \n", time_taken);
    
    return 0;
}

