#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// in base64 the alphabet has 64 words, so you need log_2(64) = 6 bits to represent them
static const char encoding_table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/',
};

static const char decoding_table[] = {
  ['A'] = 0x0,  ['B'] = 0x1,  ['C'] = 0x2,  ['D'] = 0x3,  ['E'] = 0x4,  
  ['F'] = 0x5,  ['G'] = 0x6,  ['H'] = 0x7,  ['I'] = 0x8,  ['J'] = 0x9,  
  ['K'] = 0xa,  ['L'] = 0xb,  ['M'] = 0xc,  ['N'] = 0xd,  ['O'] = 0xe,  
  ['P'] = 0xf,  ['Q'] = 0x10, ['R'] = 0x11, ['S'] = 0x12, ['T'] = 0x13, 
  ['U'] = 0x14, ['V'] = 0x15, ['W'] = 0x16, ['X'] = 0x17, ['Y'] = 0x18, 
  ['Z'] = 0x19, ['a'] = 0x1a, ['b'] = 0x1b, ['c'] = 0x1c, ['d'] = 0x1d, 
  ['e'] = 0x1e, ['f'] = 0x1f, ['g'] = 0x20, ['h'] = 0x21, ['i'] = 0x22, 
  ['j'] = 0x23, ['k'] = 0x24, ['l'] = 0x25, ['m'] = 0x26, ['n'] = 0x27, 
  ['o'] = 0x28, ['p'] = 0x29, ['q'] = 0x2a, ['r'] = 0x2b, ['s'] = 0x2c, 
  ['t'] = 0x2d, ['u'] = 0x2e, ['v'] = 0x2f, ['w'] = 0x30, ['x'] = 0x31, 
  ['y'] = 0x32, ['z'] = 0x33, ['0'] = 0x34, ['1'] = 0x35, ['2'] = 0x36, 
  ['3'] = 0x37, ['4'] = 0x38, ['5'] = 0x39, ['6'] = 0x3a, ['7'] = 0x3b, 
  ['8'] = 0x3c, ['9'] = 0x3d, ['+'] = 0x3e, ['/'] = 0x3f,
};

char* base64_encode(char* input) {
  int input_len = strlen(input);
  // 8 * n / 6 -> 4 * n / 3 gives unpadded length
  // 8 ASCII bits * input_length / 6 base64 bits
  // And round up to the nearest multiple of 4 for padding,
  // and as 4 is a power of 2 can use bitwise logical operations.
  // ((4 * n / 3) + 3) & ~3
  int size = ((4 * input_len / 3) + 3) & ~3;
  char* output = malloc(size);
  unsigned char bits[size * 8];

  // convert input to bits
  int k = 0;
  for (int i = 0; i < input_len; i++) {
    for (int j = 7; j >= 0; j--) {
      bits[k++] = (((unsigned char)input[i] >> j) & 1);
    }
  }
  for (int i = input_len; i < size; i++) {
    bits[k++] = 0;
  }

  unsigned char subbits[6] = {0};
  int bits_len = input_len * 8;
  k = 0;
  for (int start = 0; start < bits_len; start += 6) {
    // getting the subbits
    for (int i = start; i < (start + 6); i++) {
      subbits[i - start] = bits[i];
    }

    // convert the binary number to decimal
    int decimal = 0;
    for (int i = 5; i >= 0; i--) {
      // Multiply the bit by its positional value (2^position)
      // and add it to the decimal value
      decimal += subbits[i] * (1 << (5 - i));
    }
    output[k++] = encoding_table[decimal];
  }
  while (k < size) {
    output[k++] = '=';
  }
  return output;
}

char* base64_decode(char* input) {
  // char *output = malloc(sizeof(input));
  printf("decoding: %s\n", input);

  int input_len = strlen(input);
  unsigned char bits[input_len * 6];
  int bits_len = strlen(input) * 6;

  // convert input to bits
  int k = 0;
  for (int i = 0; i < input_len; i++) {
    int decimal = decoding_table[input[i]];
    while (decimal > 0) {
      // bits[k] = decimal % 2;
      printf("%d", decimal % 2);
      decimal = decimal / 2;
      k++;
    }
    printf("\n");
  }
  printf("\n");

  // printf("\n\nconverted bits:\n");

  // for (int k = 0; k < bits_len; k++) {
  //   printf("%d\n", bits[k]);
  // }
  // printf("\n");
  

  // int k = 0;
  // for (int i = 0; i < input_len; i++) {
  //   for (int j = 7; j >= 0; j--) {
  //     printf("%d", (((unsigned char)input[i] >> j) & 1));  // print each bit
  //     bits[k++] = (((unsigned char)input[i] >> j) & 1);
  //   }
  // }

  // int bits_len = size *8;
  // unsigned char subbits[8] = {0};
  // for (int start = 0; start<size; start+=8) {
  //   // getting the subbits
  //   for (int i = start; i < (start + 8); i++) {
  //     subbits[i - start] = bits[i];
  //     printf("copio %c\n", bits[i]);
  //   }

  //   for(int k = 0; k < 8; k++) {
  //     printf("%c", subbits[k]);
  //   }
  //   printf("\n");

  //   // convert the binary number to decimal
  //   int decimal = 0;
  //   for (int i = 7; i >= 0; i--) {
  //     // Multiply the bit by its positional value (2^position)
  //     // and add it to the decimal value
  //     decimal += bits[i] * (1 << (5 - i));
  //   }
  //   printf("decimal: %d\n", decimal);
  // }
  return "";
}

int main() {
  char* to_encode = "Hello World";
  printf("Encoding string: %s\n", to_encode);
  char* encoded = base64_encode(to_encode);
  printf("Encoded string: %s\n", encoded);
  char* decoded = base64_decode(encoded);
  free(encoded);
  return 0;
}