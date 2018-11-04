#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define CAPACITY 1
#define PRINT_TAB 3

//create binary node
typedef struct bNode {
    short letter;
    struct bNode *left;
    struct bNode *right;
} BNode;

//create heap node
typedef struct hNode {
  short letter;
  unsigned long long freq;
  BNode * node;
} HNode;

//create heap
typedef struct heap {
    size_t capacity;
    size_t size;
    HNode **data;
} Heap;

//heap functions
Heap * createHeap();
void destroy(Heap *heap);
void push(Heap *heap, HNode *node);
void resize(Heap *heap);
void print(Heap *pq);
HNode *peek(Heap *heap);
HNode *pop(Heap *heap);
bool is_power_two(int x);
void sift(Heap *heap, size_t ind);

int is_in(Heap *heap, char letter);
Heap *get_freqs(const char *filename);

void clear(BNode **BNode); //binary

BNode *form_tree(Heap *heap);
void print_tree(BNode *root, int depth);
void fill_encodings(BNode *root, char  **encodings, char *curr_code);
char **create_map();

void compress(FILE * input, char ** encodings, FILE * output);
void write_to_file(char * bits, FILE * output);
void free_encodings(char ** encodings);

int main(int argc, char *argv[]) {
    // Read file
    // Get frequency of every char
    // Create ordered list of nodes that give letter frequencies
    if (argc == 1) {
        printf("No file argument passed. Exiting.\n");
        return 1;
    }

    const char *filename = (const char *) argv[1];
    char * outname = NULL;

    //puts("filename");

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
      strcat(outname, ".huf");
      //printf("%s\n", outname);
    }

    FILE * input;
    input = fopen(filename, "r");
    FILE * output;
    output = fopen(outname, "w");

    //get frequencies of file
    Heap *heap = get_freqs(filename);
    print(heap);
    rewind(input);

    //store nodes into tree
    BNode *huff_tree = form_tree(heap);
    print_tree(huff_tree, 0);
    //puts("printing the heap");
    //print(heap);

    //create encodings
    char **encodings = create_map();
    fill_encodings(huff_tree, encodings, "");
    for (int i = 0; i < 256; ++i) {
        if (encodings[i] != 0) {
            printf("Encoding for %c: %s\n", i-1, encodings[i]);
        }
    }

    //compress file by encodings
    compress(input, encodings, output);
    fclose(input);
    fclose(output);

    free_encodings(encodings);
    destroy(heap);
    return 0;
}

//create heap
Heap *createHeap () {
    Heap *heap = malloc(sizeof(Heap));
    heap->size = 0;
    heap->capacity = CAPACITY;
    heap->data = calloc(CAPACITY, sizeof(HNode *));
    return heap;
}

//destroy heap
void destroy (Heap *heap) {
    //heap->data = calloc(heap->capacity, sizeof(unsigned int));
    heap->size = 0;
    heap->capacity = 0;
    free(heap->data);
    free(heap);
}

//insertion into priority queue
void push (Heap *heap, HNode *node) {// this is what's changed for priority queues
    resize(heap);
    heap->data[heap->size++] = node;

    for (size_t i = heap->size - 1, pi = (i - 1) / 2; i > 0 && heap->data[pi]->freq > heap->data[i]->freq; i = pi, pi = (pi - 1) / 2) {
        HNode *tmp = heap->data[i];
        heap->data[i] = heap->data[pi];
        heap->data[pi] = tmp;
    }
}

//resize heap as necessary
void resize (Heap *heap) {
    if (heap->size >= heap->capacity-1) {
        heap->capacity *= 2;
        heap->data = realloc(heap->data, sizeof(HNode *) * heap->capacity);
    }
}

//print priority queue
void print(Heap *pq) {
    short curr;
    for (size_t i = 0, count = 0; i < pq->size; count++) {
        // Print out layer
        for (size_t j = 0; j < count; j++) {
            for (int k = 0; k < PRINT_TAB; k++) {
                printf(" ");
            }
        }
        while (!is_power_two(i+2)) {
            if (i >= pq->size) {
                puts("\n");
                return;
            }
            curr = pq->data[i]->letter;
            if (curr != ' ' && curr != '\n')
                printf("%c:%llu ", curr, pq->data[i]->freq);
            else if (curr == '\n')
                printf("\\n:%llu ", pq->data[i]->freq);
            else
                printf("\\t:%llu ", pq->data[i]->freq);
            i++;
        }
        if (i == pq->size) {
            puts("");
            return;
        }
        curr = pq->data[i]->letter;
        if (curr != ' ' && curr != '\n')
            printf("%c:%llu\n", curr, pq->data[i]->freq);
        else if (curr == '\n')
            printf("\\n:%llu\n", pq->data[i]->freq);
        else
            printf("\\t:%llu\n", pq->data[i]->freq);
        i++;
    }
    puts("");
}

//return next node to get deleted
HNode * peek (Heap *heap) {
    return heap->data[0];
}

//delete next node
HNode * pop(Heap *heap) {
    HNode *out = heap->data[0];
    heap->data[0] = heap->data[--heap->size];

    sift(heap, 0);
    return out;
}

//power function
bool is_power_two(int x) {
    while (x != 1) {
        if (x%2 == 1) {
            return false;
        } else {
            x /= 2;
        }
    }
    return true;
}

//sift function after deletion
void sift(Heap *hp, size_t index) {
  size_t max = index;
  HNode *tmp;
  for (size_t i = max; i*2+1 < hp->size; i = max) {
      max = i;
      if (hp->data[max]->freq > hp->data[i*2+1]->freq) {
          max = i*2+1;
      }
      if (i*2+2 < hp->size && hp->data[max]->freq > hp->data[i*2+2]->freq) {
          max = i*2+2;
      }
      if (max == i)
          break;
      else {
          tmp = hp->data[i];
          hp->data[i] = hp->data[max];
          hp->data[max] = tmp;
      }
  }

}

//checks if letter in heap
int is_in(Heap *heap, char letter) {
    for (int i = 0; i < (int) heap->size; ++i) {
        if (heap->data[i]->letter == letter) {
            return i;
        }
    }
    return -1;
}

//create node for every letter
Heap *get_freqs(const char *filename) {
  Heap * out = createHeap();
  FILE *fp;
  fp = fopen(filename, "r");
  char curr;
    //makes sure that first character gets node made
    do {
      curr = fgetc(fp);
      //checks if letter is in heap
      int found = is_in(out, curr);
      if (found != -1) {
        //if node exists, add to frequency & shift heap accordingly
          out->data[found]->freq++;
          sift(out, found);
      } else {
        //create new node
          HNode *new = malloc(sizeof(HNode));
          new->letter = curr;
          new->freq = 1;
          new->node = NULL;
          push(out, new);
      }
      //print(out);
    }
    while (!feof(fp));

    print(out);

    fclose(fp);
    return out;
}

//clear the binary tree
void clear(BNode **root) {
    if (!(*root)) {
        return;
    } else {
        clear(&((*root)->left));
        clear(&((*root)->right));
        free(*root);
        *root = NULL;
    }
}

//print the binary tree
void print_tree(BNode *root, int depth) {
    if (!root) {
        puts("");
        return;
    }
    print_tree(root->right, depth+1);
    for (int i = 0; i < depth; ++i) {
        printf("   ");
    }
    if (root->letter != '\n')
      printf("%c\n", root->letter);
    else
      printf("\\n\n");
    print_tree(root->left, depth+1);
}

//creates tree by popping off heap and adding to binary tree
BNode *form_tree(Heap *heap) {
  HNode *hRoot, *leftHNode, *rightHNode;
  BNode *bRoot;
  while(heap->size > 1) {
      leftHNode = pop(heap);
      rightHNode = pop(heap);
      printf("leftHNode: %d frequency: %llu\n", (char) leftHNode->letter, leftHNode->freq);
      printf("rightHNode: %d frequency:%llu\n", (char) rightHNode->letter, rightHNode->freq);
      if(leftHNode->node == NULL) {
          leftHNode->node = malloc(sizeof(BNode));
          leftHNode->node->letter = leftHNode->letter;
          leftHNode->node->left = NULL;
          leftHNode->node->right = NULL;
      }
      if(rightHNode->node == NULL) {
          rightHNode->node = malloc(sizeof(BNode));
          rightHNode->node->letter = rightHNode->letter;
          rightHNode->node->left = NULL;
          rightHNode->node->right = NULL;
      }
      bRoot = malloc(sizeof(BNode));
      bRoot->left = leftHNode->node;
      bRoot->right = rightHNode->node;
      bRoot->letter = '\0';
      hRoot = malloc(sizeof(HNode));
      hRoot->freq = leftHNode->freq + rightHNode->freq;
      hRoot->node = bRoot;
      hRoot->letter = '\0';
      push(heap, hRoot);
      free(leftHNode);
      free(rightHNode);
  }
  return bRoot;
}

//adding encodings to each node
void fill_encodings(BNode *root, char  **encodings, char *curr_code) {
    if (root == NULL) return;

    if (root->left || root->right) {
        char *left_code = malloc((strlen(curr_code)+2)*sizeof(char));
        char *right_code = malloc((strlen(curr_code)+2)*sizeof(char));
        strcpy(left_code, curr_code);
        strcpy(right_code, curr_code);
        strcat(left_code, "0");
        strcat(right_code, "1");
        fill_encodings(root->left, encodings, left_code);
        fill_encodings(root->right, encodings, right_code);
        free(root);
    } else {
        encodings[(size_t)root->letter+1] = curr_code;
        free(root);
    }
}

//create array for encodings and sets to null
char **create_map() {
    char **encodings = malloc(sizeof(char *) * 257);
    for (size_t i = 0; i<257; ++i) {
        encodings[i] = NULL;
    }
    return encodings;
}

void free_encodings(char ** encodings) {
  for(size_t i = 0; i < 257; ++i) {
    if(encodings[i] != NULL)
      free(encodings[i]);
  }
  free(encodings);
}

//write byte to file
void write_to_file(char * bits, FILE * output){
    static unsigned char buffer = 0;
    static unsigned int size = 0;
    for (size_t i = 0; bits[i]!='\0'; ++i){
      buffer = buffer << 1;
      if(bits[i] == '1')
        buffer++;
      size++;
      if(size == 8) {
        fputc(buffer, output);
        size = 0;
        buffer = 0;
        fflush(output);
      }
    }
}

//write to compressed file
void compress(FILE * input, char ** encodings, FILE * output) {
  //header
  //count number of encodings and put into file
  unsigned short count = 0;
  for(size_t i = 0; i < 257; ++i) {
    if(encodings[i] != NULL)
      count++;
  }

  fputc(count>>8, output);
  fputc((count<<8)>>8, output);

  //8 bits = 1 byte & 9th char should be null character
  char str[9];
  str[8] = '\0';
  size_t tmp;

  for(size_t i = 0; i < 256; ++i) {
    //writing strings - writes character, length (bits), encoding itself
    if(encodings[i] != NULL) {
      tmp = i;
      for(size_t j = 7; (int) j >= 0; --j) {
        str[j] = (char) (tmp % 2) + '0';
        tmp /= 2;
      }
      write_to_file(str, output);

      //put count in string
      unsigned short count;
      for(count = 0; encodings[i][count] != '\0'; ++count);
      for(size_t j = 7; (int) j >= 0; --j) {
          str[j] = (char) (count % 2) + '0';
          count /= 2;
      }
      write_to_file(str, output);
      write_to_file(encodings[i], output);
    }
  }

  short curr;
  do {
    curr = fgetc(input);
    write_to_file(encodings[(size_t)curr+1], output);
  }
  while(!feof(input));

  //write EOF file
  for(size_t i = 0; i < 8; ++i) {
    str[i] = '0';
  }
  write_to_file(str, output);
}
