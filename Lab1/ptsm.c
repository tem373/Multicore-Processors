#include <stdio.h>
#include <stdlib.h>
//#include <omp.h>

// Global variables
int cost = 0;
int num_cities = 0;
int cost_matrix[10][10];    // max 10 cities
int visited_cities[10];     // max 10 cities

/********************************** HELPER FUNCTIONS ********************************/
int tsp(int c) {//, int cost_matrix[][num_cities-1], int visited_cities[]) {
    int count, nearest_city = 999;
    int minimum = 999, temp;
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

void minimum_cost(int city) {//, int cost_matrix[][num_cities-1], int visited_cities[]) {
    int nearest_city;
    visited_cities[city] = 1;
    printf("%d ", city);
    nearest_city = tsp(city);//, cost_matrix, visited_cities);
    if(nearest_city == 999) {
        return;
    }
    minimum_cost(nearest_city); //, cost_matrix, visited_cities);
}

/*********************************** MAIN FUNCTION **********************************/
int main(int argc, char* argv[]) {
    // Parse command-line arguments and check for correct usage
    if(argc != 4) {
        printf("Incorrect number of arguments provided - please provide arguments as detailed below:\n");
        printf("./ptsm x t inputfile.txt\n");
        exit(1);
    }

    // Initialize variables + assign input numbers
    num_cities = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    //int cost_matrix[num_cities-1][num_cities-1];
    //int visited_cities[num_cities-1];  // Optimal route will be stored here

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
    printf("Best path: ");
    minimum_cost(0); //, cost_matrix, visited_cities);  // Start at city '0' by convention
    printf("\n");
    printf("Distance: %d\n", cost);

    return 0;
}

