/******************************************************************************
 * File Name: mmsv.c
 * Name: Ruhan Randula Nanayakkara
 * Student ID: 23080386
 * Course: Operating Systems - COMP2006
 * Date: 4/27/2026
 * Description: Program Reads an n x n matrix from a file and uses concurrent threads
 * to validate if it is a magic square by checking row, column, diagonal sums, and uniqueness.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // for threading
#include <unistd.h> // for sleep function

//Global Variables
int n; // This is for the size of the matrix
int **matrix; // This is for the matrix itself
int score = 0; // This is for the score of the magic square
pthread_mutex_t score_mutex; // Mutex variable to protect the score variable during concurrent updates by multiple threads.

// Status flags to track the validation results for rows, columns, and diagonals and uniquess to include them in the report.
int *row_status;
int *col_status;
int main_diag_status = 0;
int sec_diag_status = 0;
int uniqueness_status = 0;

//-----------------
//Thread Functions:
    //1. Row Validation Thread Function
    void *validate_row(void *arg) {
        int tid= *((int *)arg); // Get the thread ID (row index) from the argument then dereference it to get the actual value.
        int expected_sum = n * (n * n + 1) / 2; // Calculate the expected sum for a magic square of size n using the magic constant formula n(n^2 + 1)/2

        //loop through the rows
        for (int i = 0; i < n; i++){
            int current_r_sum = 0; // Initialize the current sum for the row to 0.

            //calculate current row sum
            for (int j = 0; j < n; j++) {
                current_r_sum += matrix[i][j]; // Add the value of the current element in the row to the current row sum.
            }   

            //add artifical as requested to simulate heavy processing and test sychronization robustness
            sleep(1); // Sleep for 1 second to simulate heavy processing and test synchronization robustness.

            //Critical section to update the score and status flags
            pthread_mutex_lock(&score_mutex); // Lock the mutex to ensure mutual exclusion to the score variable and status flags.

            if (current_r_sum == expected_sum) { // Check if the current row sum matches the expected sum for a magic square.
                row_status[i] = 1; // If it matches, set the status for this row to 1 (valid).
                score += 1; // will increment the global score
            } else {
                row_status[i] = -1; // If it does not match, set the status
                if (score > 0) {
                    score -= 1; // Decrement the score if it was previously incremented for this row.
                }
            
            }

            pthread_mutex_unlock(&score_mutex); // Unlock the mutex after updating the score and status flags.

           
        }

        //Print the status of the row validation for this thread as requested.
        printf("Thread ID-%d: Row checks completed.\n", tid);
        //Exit the thread
        pthread_exit(NULL);
    }

    //2. Column Validation Thread Function
    void *validate_col(void *arg) {
        int tid = *((int *)arg); 
        int expected_sum = n * (n * n + 1) / 2; 

        //loop starting with j because we are checking cols.
        for (int j = 0; j < n; j++) {
            int current_c_sum = 0;

            for (int i = 0; i < n; i++) {
                current_c_sum += matrix[i][j];
            }

            sleep(1); // Simulate heavy processing

            // Critical section to update the score and status flags
            pthread_mutex_lock(&score_mutex);

            if (current_c_sum == expected_sum) {
                col_status[j] = 1;
                score += 1;
            } else {
                col_status[j] = -1;
                if (score > 0) {
                    score -= 1;
                }
            }

            pthread_mutex_unlock(&score_mutex);
            //Crtical Section end

            
        }
        printf("Thread ID-%d: Column checks completed.\n", tid);
        pthread_exit(NULL);

    }

    //3: Diagonal Validation Thread Function
    void *validate_diag(void *arg) {
        int tid = *((int *)arg);
        int expected_sum = n * (n * n + 1) / 2;
        
        int main_diag_sum = 0;
        int sec_diag_sum = 0;

        //Calculating both diagonals
        for (int i = 0; i < n; i++) {
            main_diag_sum += matrix[i][i]; // Main diagonal elements are at positions (i, i)
            sec_diag_sum += matrix[i][n - 1 - i]; // Secondary diagonal elements are at positions (i, n-1-i)
        }

        sleep(1); // Simulate heavy processing

        //Critical Secion Start
        pthread_mutex_lock(&score_mutex);

        //Main Diagonal Check
        if (main_diag_sum == expected_sum) {
            main_diag_status = 1;
            score += 1;
        } else {
            main_diag_status = -1;
            if (score > 0) {
                score -= 1;
            }
        }

        //Secondary Diagonal Check
        if (sec_diag_sum == expected_sum) {
            sec_diag_status = 1;
            score += 1;
        } else {
            sec_diag_status = -1;
            if (score > 0) {
                score -= 1;
            }
        }

        pthread_mutex_unlock(&score_mutex);
        //Critical Section End

        printf("Thread ID-%d: Diagonal checks completed.\n", tid);
        pthread_exit(NULL);
    }

    //4: Uniqueness Validation Thread Function
    void *validate_uniqueness(void *arg) {
        int tid = *((int *)arg);
        int max_val = n * n; // The maximum value in a magic square of size n is n^2
        int is_valid = 1; // Flag to track if the uniqueness condition is satisfied

        // Create a boolean array to track the presence of numbers from 1 to n^2
        int *checked = (int *)calloc(max_val + 1, sizeof(int)); // Allocate an array of size n^2 + 1 initialized to 0
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int val = matrix[i][j];
                if (val < 1 || val > max_val || checked[val] == 1) { // Check if the value is out of range or already seen
                    is_valid = 0; // If invalid, set the flag to 0
                    break;
                }
                checked[val] = 1; // Mark the value as seen
            }
            if (!is_valid) {
                break; // Exit early if uniqueness condition is violated
            }
        }
        free(checked); // Free the dynamically allocated memory
        sleep(1); // Simulate heavy processing

        //Critical Section Start
        pthread_mutex_lock(&score_mutex);
        if (is_valid) {
            uniqueness_status = 1;
            score += 1;
        } else {
            uniqueness_status = -1;
            if (score > 0) {
                score -= 1;
            }
        }
        pthread_mutex_unlock(&score_mutex);
        //Critical Section End

        printf("Thread ID-%d: Uniqueness checks completed.\n", tid);
        pthread_exit(NULL);
    }
//-----------------
//Main Function
int main(int argc, char *argv[]) {
    //1. First check the command line arguments to ensure a filename is provided and handle the case where it is not provided or incorrect.
    if (argc != 2) { //!=2 means that the program expects exactly one argument (the filename), in addition to the program name itself.
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    //2. Open the input file in read mode.
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    //3. Read the matrixsize from the file 
    if (fscanf(file, "%d", &n) != 1) {
        printf("Error: Could not read matrix size\n");
        return 1;
    }

    //4. Allocating the memory for the matrix dynamically
    matrix = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        matrix[i] = (int *)malloc(n * sizeof(int));
    }

    //5. Dynamically allocate memory for status flags based on the size of n
    row_status = (int *)malloc(n * sizeof(int));
    col_status = (int *)malloc(n * sizeof(int));

    //6. Initalize the status arryas to pending
    for (int i = 0; i < n; i++) {
        row_status[i] = 0;
        col_status[i] = 0;
    }

    //7. Read matrix data from the input file
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++) {
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                printf("Error: Could not read matrix data\n");
                return 1;
            }
        }
    }
    // close the file
    fclose(file);

    //8. Print the matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

    printf("Inital Score: %d\n", score);

    //Initalize the mutex
    if (pthread_mutex_init(&score_mutex, NULL) != 0) {
        printf("Error: Could not initialize mutex");
        return 1;
    }

    //9. Create threads for each validation function
    // Create thread IDs for each validation thread
    pthread_t row_thread, col_thread, diag_thread, uniqueness_thread; 
    int row_tid = 1, col_tid = 2, diag_tid = 3, uniqueness_tid = 4; // Thread IDs for identification in print statements
    // Create threads for each validation function
    pthread_create(&row_thread, NULL, validate_row, &row_tid);
    pthread_create(&col_thread, NULL, validate_col, &col_tid);
    pthread_create(&diag_thread, NULL, validate_diag, &diag_tid);
    pthread_create(&uniqueness_thread, NULL, validate_uniqueness, &uniqueness_tid);

    //10. Wait for all threads to complete using pthread_join
    
    pthread_join(row_thread, NULL);
    pthread_join(col_thread, NULL);
    pthread_join(diag_thread, NULL);
    pthread_join(uniqueness_thread, NULL);
    

    //11. Print the Report
    printf("\nValidation Report:\n");

    //check Rows
    int valid_rows = 1;
    printf("Row Status:\n");
    for (int i = 0; i < n; i++) {
        if (row_status[i] == -1){
            printf("Row %d: Invalid\n", i + 1); // Print the row number (i + 1) and its status (Invalid)
            valid_rows = 0; // Set the flag to indicate that not all rows are valid
        }
    }

    if (valid_rows) {
        printf("All Valid.\n");
    }

    //check Columns
    int valid_cols = 1;
    printf("Column Status:\n");
    for (int j = 0; j < n; j++) {
        if (col_status[j] == -1) {
            printf("Column %d: Invalid\n", j + 1); // Print the column number (j + 1) and its status (Invalid)
            valid_cols = 0; // Set the flag to indicate that not all columns are valid
        }
    }

    if (valid_cols) {
        printf("All Valid.\n");
    }

    // Check Diagonals
    printf("Diagonal Status:\n");
    if (main_diag_status == 1 && sec_diag_status == 1) {
        printf("All Valid\n");
    } else {
        if (main_diag_status == -1) printf("Main Diag Invalid ");
        if (sec_diag_status == -1) printf("Secondary Diag Invalid ");
        printf("\n");
    }

    //Check Uniqueness
    printf("Uniqueness Status:\n");
    if (uniqueness_status == 1) {
        printf("All Valid\n");
    } else {
        printf("Invalid\n");
    }

    //Calculate final max score
    int max_score = 2 * n + 3; // (n rows + n cols + 2 diagonals + 1 uniqueness check)
    printf("Final Score: %d out of %d\n", score, max_score);

    //Final Result whether valid or invalid
    if (score == max_score) {
        printf("RESULT: The matrix is a valid magic square.\n");
    } else {
        printf("RESULT: The matrix is not a valid magic square.\n");
    }
    //-------
    //clean up dynamically allocated memory to ensure no memory leaks
    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(row_status);
    free(col_status);

    //cleanup mutex
    pthread_mutex_destroy(&score_mutex);

    return 0;// exit
}
