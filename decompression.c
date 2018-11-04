#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

void decompress(FILE * input, FILE * output);
void header(FILE * input, short * letters, short * lengths, short * encodings, short * buffer, short total);
void buffer_complete(short **buffer, short value);
void body_comparison(short * letters, short * lengths, short * encodings, short * buffer, short total, FILE * input, FILE * output);
short compare_vals(short *arr1, short *arr2,  short size, short val1, short val2);
void free_arrays(short * letters, short * lengths, short * encodings, short * buffer);

short pos_of_buffer = 0;

int main(int argc, char * argv[]) {
  if (argc == 1) {
      printf("No file argument passed. Exiting.\n");
      return 1;
  }

  const char *filename = (const char *) argv[1];
  char * outname = NULL;

  if(argc == 3) {
    outname = malloc((sizeof(strlen(argv[2])) + 1) + sizeof(char));
    strcat(outname, argv[2]);
    //puts("three args");
  } else if(argc == 2) {
    outname = malloc((strlen(filename) + 5) * sizeof(char));
    //puts("two args");
    strcpy(outname, filename);
    //puts("HELLO");
    //printf("%s\n", outname);
    strcat(outname, ".txt");
    //printf("%s\n", outname);
  }

  FILE * input;
  input = fopen(filename, "r");
  FILE * output;
  output = fopen(outname, "w");

  decompress(input, output);

  fclose(input);
  fclose(output);
}

void decompress(FILE * input, FILE * output) {
  //defines encoding total from first two bytes
  short total = (fgetc(input) << 8) + fgetc(input) - 1;
  //printf("%d\n",total);

  //creates three maps to be similar to compress functions
  short * letters = malloc(sizeof(short) * (total+1));
  short * lengths = malloc(sizeof(short) * (total+1));
  short * encodings = malloc(sizeof(short) * (total+1));

  //create buffer and use it as place holder for empty bits
  short * buffer = malloc(sizeof(short) * 8);
  buffer_complete(&buffer, fgetc(input));

  //decipher the header
  header(input, letters, lengths, encodings, buffer, total);

  //go through actual compression code and compare to encodings
  body_comparison(letters, lengths, encodings, buffer, total, input, output);

  //free arrays
  free_arrays(letters, lengths, encodings, buffer);
}

void header(FILE * input, short * letters, short * lengths, short * encodings, short * buffer, short total) {
  //for each encodings
  for(short i = 0; i < total + 1; ++i) {
      short hold = 0;
      //store ascii values
      for(short j = 0; j < 8; ++j) {
          //parts of storage of ascii values
          hold = hold * 2 + buffer[pos_of_buffer++];
          if(pos_of_buffer == 8)
            buffer_complete(&buffer, fgetc(input));
      }
      letters[i] = hold; //store values for ascii values in letter array

      ///for each length of encodings
      short length = 0;
      for(short j = 0; j < 8; ++j) {
          //bit shifting
          length = length * 2 + buffer[pos_of_buffer++];
          if(pos_of_buffer == 8)
            buffer_complete(&buffer, fgetc(input));
      }
      lengths[i] = length;

      //for each actual encoding (from compression)
      short encoding = 0;
      for(short j = 0; j < length; ++j) {
          //manipulate encoding
          encoding = encoding * 2 + buffer[pos_of_buffer++];
          if(pos_of_buffer==8)
            buffer_complete(&buffer, fgetc(input));
      }
      encodings[i] = encoding;
  }
}

//go throguh encodings
void body_comparison(short * letters, short * lengths, short * encodings, short * buffer, short total, FILE * input, FILE * output) {
  short count = 1, val = 0, len = 0;
  while(count == 1) {
      val = 0;
      len = 0;

      while(compare_vals(encodings, lengths, total + 1, val, len) == 0) {
          //bitshift and increment the buffer
          val = (val * 2) + buffer[pos_of_buffer++];
          len++;
          //complete buffer
          if(pos_of_buffer == 8)
            buffer_complete(&buffer, fgetc(input));
      }

      //store letter and place in file
      short letter = letters[compare_vals(encodings, lengths, total + 1, val, len)] - 1;
      fputc((char) letter, output);
      //if EOF, break loop
      if(letter == -1 || letter == 10)
        count = 0;
      //else printf("%c", letter);
  }
//  printf("\n");
}

//fill in buffer
void buffer_complete(short **buffer, short value) {
    for(short i = 0; i < 8; ++i) {
        (*buffer)[7 - i] = (value % 2);
        value /= 2;
    }
    pos_of_buffer = 0;
}

//if encodings are equal
short compare_vals(short *arr1, short *arr2, short size, short val1, short val2) {
    for(short i = 0; i < size; i++)
        if(arr1[i] == val1 && arr2[i] == val2) return i;
    return 0;
}

//free everything
void free_arrays(short * letters, short * lengths, short * encodings, short * buffer) {
  free(letters);
  free(lengths);
  free(encodings);
  free(buffer);
}












/*



get bitstream so all characters are the all_data

go through encodings and separate header to make tree

go through actual encodings, find letter through tree, write to file



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define CAPACITY 1
#define PRINT_TAB 3

//bitstream
typedef struct bitstream {
  char * all_data;
  short curr_bit;
  short curr_byte;
} Bitstream;

//create binary node
typedef struct bNode {
    short letter;
    struct bNode *left;
    struct bNode *right;
} BNode;

Bitstream * create_bitstream(FILE *input);
void decompress(FILE *input, FILE *output);
BNode * create_node(short letter);
void write_to_file(char * bits, BNode * root, FILE * output);

int main(int argc, char *argv[]) {
  if(argc == 1) {
    printf("No file argument passed. Exiting.\n");
    return 1;
  }

  const char *filename = (const char *) argv[1];
  FILE * input;
  input = fopen(filename, "r");
  FILE * output;
  output = fopen("huffman_decoding.txt", "w");
  puts("Hello");

  Bitstream * bitstream = create_bitstream(input);
  puts("hellllloo");
  printf("String: %s\n", bitstream->all_data);

}

void decompress(FILE *input, FILE *output) {
  rewind(input);
  unsigned short num_of_encodings = (fgetc(input)<<8) + fgetc(input);

  short letter;

  for(size_t i = 1; i < num_of_encodings; ++i) {
    val = fgetc(input);

  }

}

Bitstream * create_bitstream(FILE * input) {
  Bitstream * bitstream = malloc(sizeof(Bitstream));

  fseek(input, 0, SEEK_END);
  size_t length = ftell(input);
  rewind(input);
  char * file_countents = malloc(length * sizeof(short) * 8);
  fread(file_countents, sizeof(short), length, input);
  printf("String: %s\n", file_countents);
  bitstream->all_data = file_countents;
  bitstream->curr_bit = 0;
  bitstream->curr_byte = 0;
}

BNode * create_node(short letter) {
  BNode * node = malloc(sizeof(BNode));
  node->letter = letter;
  node->left = NULL;
  node->right = NULL;
}

BNode * create_tree(BNode * root, short encoding) {
  BNode * holder = root;

}

//goes through root
void write_to_file(short encodings, BNode * root, FILE * output) {
  BNode * node = root;
  short val = encodings;
  while(node->left != NULL || node->right != NULL) {
    val = encodings % 2;
    encodings /= 2;
    if(val == 0) {
      node = node->left;
    } else {
      node = node->right;
    }
  }

  fputc(node->letter, output);
}

*/
