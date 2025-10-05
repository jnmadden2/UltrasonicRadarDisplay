#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]){
    int values;
    if (argc < 2) {
        //printf("Please provide an integer argument.\n");
        //return 1; // Return an error code
        values = 10;
    }
    else{
        values = atoi(argv[1]);
    }

    // Open the file for writing
    FILE *file = fopen("sampleData.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Seed the random number generator
    srand(time(0));

    // Generate random numbers and write them to the file
    for (int i = 0; i < values; i++) {
        int randomDistance = rand() % 1001;  // Generate a random integer for distance (in mm)
        int randomAngle = rand() % (130 - 50 + 1) + 50;
        fprintf(file, "%d,%d.\n", randomDistance,randomAngle);  // Write the integer to the file
    }

    // Close the file
    fclose(file);

    printf("Generated %d random numbers and saved to sampleData.txt\n", values);

    return 0;
}