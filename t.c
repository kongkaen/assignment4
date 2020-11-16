#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <pthread.h>

#define MAX_LINE 1000
#define LINES 10
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
// Initialize the mutex for buffer 1
pthread_mutex_t buffer2_mutex = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t buffer2_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer2_empty = PTHREAD_COND_INITIALIZER;

// Buffer3, shared resources between plus-replacement thread and output thread
char *buffer3[BUFF_SIZE];
// Number of items in the buffer
int buffer3_count = 0;
// Index where the plus-replacement thread will put the next item
int buffer3_prod_idx = 0;
// Index where the output thread will pick up the next item
int buffer3_con_idx = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t buffer3_mutex = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t buffer3_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer3_empty = PTHREAD_COND_INITIALIZER;

char* replaceWord(const char* s, const char* oldW,
                  const char* newW)
{
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

void put_buffer1(char *line){
  //Lock mutex1 before changing buffer1
  pthread_mutex_lock(&buffer1_mutex);

  buffer1[buffer1_prod_idx] = (char*)calloc(strlen(line), sizeof(char));
  strcpy(buffer1[buffer1_prod_idx], line);

  buffer1_prod_idx++;
  buffer1_count++;

  //Signal that buffer1 is no longer empty
  pthread_cond_signal(&buffer1_full);

  //Unlock mutex1
  pthread_mutex_unlock(&buffer1_mutex);
  return;
}

char* get_buffer1(){
  //Lock mutex1 before changing buffer1
  pthread_mutex_lock(&buffer1_mutex);

  char *get_buff1 = NULL;
  get_buff1 = (char *)malloc(1000*sizeof(char));
  memset((char*) get_buff1, '\0', sizeof(*get_buff1));
  while (buffer1_count == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&buffer1_full, &buffer1_mutex);

  strcpy(get_buff1, buffer1[buffer1_con_idx]);
  buffer1_con_idx++;
  buffer1_count--;

  //Unlock mutex1
  pthread_mutex_unlock(&buffer1_mutex);


  return get_buff1;

}

void put_buffer2(char *line){
  //Lock mutex1 before changing buffer1
  pthread_mutex_lock(&buffer2_mutex);

  buffer2[buffer2_prod_idx] = (char*)calloc(strlen(line), sizeof(char));
  strcpy(buffer2[buffer2_prod_idx], line);

  buffer2_prod_idx++;
  buffer2_count++;

  //Signal that buffer1 is no longer empty
  pthread_cond_signal(&buffer2_full);

  //Unlock mutex1
  pthread_mutex_unlock(&buffer2_mutex);
  return;
}

char* get_buffer2(){
  //Lock mutex1 before changing buffer1
  pthread_mutex_lock(&buffer2_mutex);

  char *get_buff2 = NULL;
  get_buff2 = (char *)malloc(1000*sizeof(char));
  memset((char*) get_buff2, '\0', sizeof(*get_buff2));
  while (buffer2_count == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&buffer2_full, &buffer2_mutex);

  strcpy(get_buff2, buffer2[buffer2_con_idx]);
  buffer2_con_idx++;
  buffer2_count--;

  //Unlock mutex1
  pthread_mutex_unlock(&buffer2_mutex);


  return get_buff2;

}

void put_buffer3(char *line){
  //Lock mutex1 before changing buffer1
  pthread_mutex_lock(&buffer3_mutex);

  buffer3[buffer3_prod_idx] = (char*)calloc(strlen(line), sizeof(char));
  strcpy(buffer3[buffer3_prod_idx], line);

  buffer3_prod_idx++;
  buffer3_count++;

  //Signal that buffer1 is no longer empty
  pthread_cond_signal(&buffer3_full);

  //Unlock mutex1
  pthread_mutex_unlock(&buffer3_mutex);
  return;
}

char* get_buffer3(){
  //Lock mutex1 before changing buffer1
  pthread_mutex_lock(&buffer3_mutex);

  char *get_buff3 = NULL;
  get_buff3 = (char *)malloc(1000*sizeof(char));
  memset((char*) get_buff3, '\0', sizeof(*get_buff3));
  while (buffer3_count == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&buffer3_full, &buffer3_mutex);

  strcpy(get_buff3, buffer3[buffer3_con_idx]);
  buffer3_con_idx++;
  buffer3_count--;

  //Unlock mutex1
  pthread_mutex_unlock(&buffer3_mutex);


  return get_buff3;

}

void *get_input_t(void *args){
  int terminate = 0;
  while (terminate == 0){
    char *line = NULL;
    line = (char *)malloc(MAX_LINE*sizeof(char));
    memset((char*) line, '\0', sizeof(*line));

    // Get input from stdin and store in line
    fgets(line, MAX_LINE, stdin);

    // Check if it's an empty line
    if ((line == NULL) || (strcmp(line,"STOP\n") == 0)) {
      terminate = 1;
    }
    //printf("<line> %s", line);
    put_buffer1(line);
    free(line);

  }
//  for (int i=0; i<buffer1_count;i++)
//    printf("<buffer1> %s", buffer1[i]);

  return NULL;

}

void *sepLine_t(void *args){
  int terminate =0;
  while (terminate == 0){
    char *get_buff1 = get_buffer1();

    if (strcmp(get_buff1,"STOP\n") == 0) {
        terminate = 1;
    }
    //printf("<get_buff1> %s", get_buff1);

    // Loop through each character in get_buff1
    // replace \n with space
      for (int i = 0; i < strlen(get_buff1); ++i) {
          if (get_buff1[i] == '\n'){
              get_buff1[i] = ' ';
          }
      }

      put_buffer2(get_buff1);

    //  printf("<get_buff1 after replace> %s", get_buff1);
  }
  //for (int i=0; i<buffer2_count;i++)
  //  printf("<buffer2> %s", buffer2[i]);



  return NULL;
}

void *repPlus_t(void *args){

  // Target word
  char old[] = "++";
  // word to be replace with
  char new[] = "^";

  int terminate =0;
  while (terminate == 0){

    char *get_buff2 = NULL;
    get_buff2 = (char *)malloc((1100)*sizeof(char));
    memset(get_buff2, '\0', 1100);

    get_buff2 = get_buffer2();

    if (strcmp(get_buff2,"STOP ") == 0) {
        terminate = 1;
    }

    char *plus_sign_line;

    plus_sign_line = replaceWord(get_buff2, old, new);

    put_buffer3(plus_sign_line);

  }
//  for (int i=0; i<buffer3_count;i++)
//    printf("<buffer3> %s", buffer3[i]);

  return NULL;
}

void *output_t(void *args){

  char *output_line;
  output_line = (char *)malloc((10000)*sizeof(char));
  memset(output_line, '\0', 10000);


  int total_lines = 0;

  int terminate =0;
  while (terminate == 0){

    char *line = NULL;
    line = (char *)malloc((10000)*sizeof(char));
    memset(line, '\0', 10000);

    line = get_buffer3();
    //printf("<Line> %s", line);

    // Checks for "STOP"
    if (strcmp(line,"STOP ") == 0) {
        terminate = 1;
    }
    else {
      strcat(output_line, line);
      //printf("<output> %s", line);
    }

  }

  if (strlen(output_line) >= 80) {

    // number of line
    int num_lines = strlen(output_line)/80;

    // For loop loops through the additional lines brought in
    // line then becomes the leftover chars from the last line, and the
    // new chars up to 80 +\n = 81
    for (int i = total_lines; i < num_lines; ++i) {

        char line[81];
        memset(line, '\0', sizeof(line));

        for (int c = 0; c < 80; ++c) {
            line[c] = output_line[i*80+c];
        }

        // Print the line
        printf("%s\n", line);
    }
    // Total lines now equals the num of lines and stdout is flushed
    total_lines = num_lines;
    //fflush(NULL);
  }

  return NULL;
}


int main(){

  //clock_t begin = clock();
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
