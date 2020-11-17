/*
    NATTHAPHONG KONGKAEW
    ASSIGNMENT 4 : Multi-threaded Producer Consumer Pipeline
    CS 344
    Oregon State University, Fall 2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CHAR 1000
#define CHAR_PER_LINE 80
#define BUFF_SIZE 10

// Buffer1, shared resources between input thread and line-separate thread
char *buffer1[BUFF_SIZE];
// Number of items in the buffer
int buffer1_count = 0;
// Index where the input thread will put the next item
int buffer1_prod_idx = 0;
// Index where the line-separate thread will pick up the next item
int buffer1_con_idx = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t buffer1_mutex = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t buffer1_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer1_empty = PTHREAD_COND_INITIALIZER;

// Buffer2, shared resources between line-separator thread and plus-replacement thread
char *buffer2[BUFF_SIZE];
// Number of items in the buffer
int buffer2_count = 0;
// Index where the line-separator thread will put the next item
int buffer2_prod_idx = 0;
// Index where the plus-replacement thread will pick up the next item
int buffer2_con_idx = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t buffer2_mutex = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t buffer2_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer2_empty = PTHREAD_COND_INITIALIZER;

// Buffer3, shared resources between plus-replacement thread and output thread
char *buffer3;
// Number of items in the buffer
int buffer3_count = 0;
// Index where the plus-replacement thread will put the next item
int buffer3_prod_idx = 0;
// Index where the output thread will pick up the next item
int buffer3_con_idx = 0;
// Initialize the mutex for buffer 3
pthread_mutex_t buffer3_mutex = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t buffer3_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer3_empty = PTHREAD_COND_INITIALIZER;

int output_length = 0;


// A function to Replace a word in a text by another given word
// I copied the functions from https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
char* replaceWord(const char* s, const char* oldW, const char* newW) {
    char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;

            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }

    // Making new string of enough length
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);

    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}

// Put data into buffer1
// Act as buffer1 producer
void put_buffer1(char *line){

  //Lock mutex before changing buffer1
  pthread_mutex_lock(&buffer1_mutex);

  // String copy from input line into buffer1
  buffer1[buffer1_prod_idx] = (char*)calloc(strlen(line), sizeof(char));
  strcpy(buffer1[buffer1_prod_idx], line);

  // Check length of the buffer
  //output_length += strlen(buffer1[buffer1_prod_idx]);

  // Increment buffer1 producer index
  // Increment buffer1 count
  buffer1_prod_idx++;
  buffer1_count++;

  //Signal that buffer1 is no longer empty
  pthread_cond_signal(&buffer1_full);

  //Unlock mutex
  pthread_mutex_unlock(&buffer1_mutex);

  //for (int i=0; i<buffer1_count;i++)
  //  printf("PUT BUFF 1  >> %s\n\n", buffer1[i]);
  return;
}

// Get string from buffer1
// Act as Buffer1 consumer
char* get_buffer1(){

  //Lock mutex before changing buffer1
  pthread_mutex_lock(&buffer1_mutex);

  // Get_buff1 to store data copied from buffer1
  char *get_buff1 = NULL;
  get_buff1 = (char *)malloc(MAX_CHAR*sizeof(char));
  memset((char*) get_buff1, '\0', sizeof(*get_buff1));

  while (buffer1_count == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&buffer1_full, &buffer1_mutex);

  // Copy data from buffer1
  strcpy(get_buff1, buffer1[buffer1_con_idx]);

  // Increment buffer1 consumer Index
  // Decrement buffer1 count
  buffer1_con_idx++;
  buffer1_count--;

  //Unlock mutex
  pthread_mutex_unlock(&buffer1_mutex);

  //printf("GET BUFF 1 >> %s", get_buff1);

  return get_buff1;

}

// Put data into buffer2
// Act as buffer2 producer
void put_buffer2(char *line){

  //Lock mutex before changing buffer2
  pthread_mutex_lock(&buffer2_mutex);

  // Copy data into buffer2 producer
  buffer2[buffer2_prod_idx] = (char*)calloc(strlen(line), sizeof(char));
  strcpy(buffer2[buffer2_prod_idx], line);

  // Increment buffer2 producer Index
  // Increment buffer2 count
  buffer2_prod_idx++;
  buffer2_count++;

  //Signal that to consumer that buffer2 is no longer empty
  pthread_cond_signal(&buffer2_full);

  //Unlock mutex
  pthread_mutex_unlock(&buffer2_mutex);
  //for (int i=0; i<buffer2_count;i++)
  //  printf("PUT BUFF 2  >> %s\n\n", buffer2[i]);
  return;
}

// Get data from buffer2
// Act as buffer2 consumer
char* get_buffer2(){

  //Lock mutex before changing buffer2
  pthread_mutex_lock(&buffer2_mutex);

  // Get data from buffer2 and store in get_buff2
  char *get_buff2 = NULL;
  get_buff2 = (char *)malloc(MAX_CHAR*sizeof(char));
  memset((char*) get_buff2, '\0', sizeof(*get_buff2));

  while (buffer2_count == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&buffer2_full, &buffer2_mutex);

  // Copy data from buffer2 to get_buff2
  strcpy(get_buff2, buffer2[buffer2_con_idx]);

  // Increment buffer2 consumer index
  // Decrement buffer2 count
  buffer2_con_idx++;
  buffer2_count--;

  //Unlock mutex
  pthread_mutex_unlock(&buffer2_mutex);

  //printf("GET BUFF 2 >> %s", get_buff2);

  return get_buff2;

}

// Get data from buffer3
// Act as buffer3 consumer
void put_buffer3(char *line){

  //Lock mutex before changing buffer3
  pthread_mutex_lock(&buffer3_mutex);

  // Concatenate data into buffer3
  strcat(buffer3, line);

  // number of character from input
  int num_char = strlen(line);

  // Adding number of character into buffer3 count
  buffer3_count += num_char;

  if (buffer3_count >= CHAR_PER_LINE){

    //Signal that buffer3 producer is no longer empty and have character >= 80
    pthread_cond_signal(&buffer3_full);
  }

  //Unlock mutex
  pthread_mutex_unlock(&buffer3_mutex);

  //printf("PUT BUFF 3  >> %s\n\n", buffer3);

  return;
}

// Get data from buffer3
// Act as buffer3 consumer
char* get_buffer3(){

  //Lock mutex before changing buffer3
  pthread_mutex_lock(&buffer3_mutex);

  // get_buff3 to store buff3 consumer
  char *get_buff3 = NULL;
  get_buff3 = (char *)malloc(CHAR_PER_LINE*sizeof(char));
  memset((char*) get_buff3, '\0', sizeof(*get_buff3));

  while (buffer3_count < CHAR_PER_LINE)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&buffer3_full, &buffer3_mutex);

  // Copy data into get_buffer3 as buffer3 consumer
  strcpy(get_buff3, &buffer3[buffer3_con_idx]);

  // Increment buffer3 consumer index
  // Decrement buffer3 count
  buffer3_con_idx += CHAR_PER_LINE;
  buffer3_count -= CHAR_PER_LINE;

  //Unlock mutex
  pthread_mutex_unlock(&buffer3_mutex);

  //free(get_buff3);
  //printf("GET BUFF 3  >> %s\n\n", get_buff3);

  return get_buff3;
}

// Function that get input thread gets input from stdin until "STOP" line found
// Then put input into buffer1 as buffer1 producer
void *get_input_t(void *args){

  int terminate = 0;

  // Loop until terminate value set to 1
  while (terminate == 0){

    // stdin input will store in line
    char *line = NULL;
    line = (char *)malloc(MAX_CHAR*sizeof(char));
    memset((char*) line, '\0', sizeof(*line));

    // Get input from stdin and store in line
    fgets(line, MAX_CHAR, stdin);

    // Check if it's an empty line or "STOP"
    if ((line == NULL) || (strcmp(line,"STOP\n") == 0)) {
      terminate = 1;
    }

    // put input to buffer1
    put_buffer1(line);

    //free(line);
  }

  return NULL;
}

// Function that sepLine_t thread will consume shared buffer1
// Then process replaceing '\n' character with space
// And put the result into buffer2 producer
void *sepLine_t(void *args){

  int i;
  int terminate =0;

  while (terminate == 0){

    // consume buffer1 and store in get_buff1
    char *get_buff1 = get_buffer1();

    // Check for terminating
    if (strcmp(get_buff1,"STOP\n") == 0) {
        terminate = 1;
    }

    // Loop through each character in get_buff1
    // to replace '\n' with space
      for (i = 0; i < strlen(get_buff1); ++i) {
          if (get_buff1[i] == '\n'){
              get_buff1[i] = ' ';
          }
      }

      // put result in buffer2 producer
      put_buffer2(get_buff1);

  }

  return NULL;
}

// Function that repPlus_t thread consume data from shared buffer2
// Then replace "++" with "^" and the result produce buffer3
void *repPlus_t(void *args){
    buffer3 = (char *)malloc(MAX_CHAR * sizeof(char));

  // Target word
  char old[] = "++";
  // replace with
  char new[] = "^";

  int terminate = 0;
  while (terminate == 0){

    // consume shared buffer2 and put to get_buff2
    char *get_buff2 = NULL;
    get_buff2 = (char *)malloc((MAX_CHAR)*sizeof(char));
    memset(get_buff2, '\0', MAX_CHAR);

    get_buff2 = get_buffer2();

    // Check for terminating
    if (strcmp(get_buff2,"STOP ") == 0) {
        terminate = 1;
    }

    // process the replacement in get_buff2
    char *plus_sign_line;

    // put the result into plus_sign_line
    plus_sign_line = replaceWord(get_buff2, old, new);

    // line without "++" produce buffer3
    put_buffer3(plus_sign_line);
    //free(get_buff2);
  }

  return NULL;
}

// Function that output_t thread consumes shared buffer3
// Then printing the result that have 80 character in each line
void *output_t(void *args){
  int i, character;
  // consume buffer3 into output_line
  char *output_line;
  output_line = (char *)malloc((MAX_CHAR)*sizeof(char));
  memset(output_line, '\0', MAX_CHAR);

  //printf("BUFF 3 >> %s\n\n", buffer3);
  output_line = get_buffer3();
  //printf("<OUTPUT_LINE> %s\n\n", output_line);

  // total line needed to be printing
  int total_lines = 0;

  // If output line is >= 80, print to screen
  if (strlen(output_line) >= CHAR_PER_LINE) {

    // number of lines that will be printing
    int num_lines = strlen(output_line)/CHAR_PER_LINE;

    // print each line
    for (i = total_lines; i < num_lines; ++i) {
                // each line has 80 character + '\n' = 81 total
                char print_line[CHAR_PER_LINE + 1];
                memset(print_line, '\0', sizeof(print_line));

                for (character = 0; character < CHAR_PER_LINE; ++character) {
                    print_line[character] = output_line[i*CHAR_PER_LINE+character];
                }

                // Print the line
                printf("%s\n", print_line);
    }

    // total line is now the number of lines
    total_lines = num_lines;

  }

  exit(1);

  return NULL;
}


int main(){

  pthread_t input, separate, replace, output;

  // Create the threads
  pthread_create(&input, NULL, get_input_t, NULL);
  pthread_create(&separate, NULL, sepLine_t, NULL);
  pthread_create(&replace, NULL, repPlus_t, NULL);
  pthread_create(&output, NULL, output_t, NULL);

  // Wait for the threads to terminate
  pthread_join(input, NULL);
  pthread_join(separate, NULL);
  pthread_join(replace, NULL);
  pthread_join(output, NULL);

  return EXIT_SUCCESS;
}
