#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// in base64 the alphabet has 64 words, so you need log_2(64) = 6 bits to represent them
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

char* base64_encode(char* input) {
  // int padding_size = (4 - (strlen(input) % 4)) % 4;
  // printf("padding size: %d\n", padding_size);

  // ASCII:     a         b
  // bits:   01100001  01100010
  // bits:   011000 010110 0010
  // bits:   011000 010110 001000
  // base64:   Y      W      I       =

  //    a
  // 01100001
  // 011000 01
  // 011000 010000
  

  // 8 * n / 6 -> 4 * n / 3 gives unpadded length
  // 8 ASCII bits * input_length / 6 base64 bits
  // And round up to the nearest multiple of 4 for padding, 
  // and as 4 is a power of 2 can use bitwise logical operations. 
  // ((4 * n / 3) + 3) & ~3
  int size = ((4 * strlen(input) / 3) + 3) & ~3;

  char* output = malloc(size);
  uint8_t bits[size * 8];

  // convert input to bits
  int k = 0;
  for (int i = 0; i < strlen(input); i++) {
    for (int j = 7; j >= 0; j--) {
      printf("%d", (((unsigned char)input[i] >> j) & 1));  // print each bit
      bits[k] = (((unsigned char)input[i] >> j) & 1);
      k++;
    }
  }
  printf("\n");

  uint8_t subbits[6] = {0};
  int bits_len = strlen(input) * 8;
  k = 0;
  for (int start = 0; start < bits_len; start += 6) {
    // getting the subbits
    printf("getting subbits from %d to %d\n", start, start + 6);
    for (int i = start; i < (start + 6); i++) {
      printf("inserting into subbits[%d] = bits[%d] = %d\n", (i - start), i, bits[i]);
      subbits[i - start] = bits[i];
    }

    // convert the binary number to decimal
    int decimal = 0;
    for (int i = 5; i >= 0; i--) {
      // Multiply the bit by its positional value (2^position)
      // and add it to the decimal value
      decimal += subbits[i] * (1 << (5 - i));
    }
    printf("encoding_table[%d] = %c\n", decimal, encoding_table[decimal]);
    output[k++] = encoding_table[decimal];
  }
  return output;
}

int main() {
  // char* to_encode = "Hello World!";
  char* to_encode = "Hello World";
  // char* to_encode = "a";
  printf("Encoding string: %s\n", to_encode);
  char* encoded = base64_encode(to_encode);
  printf("Encoded string: %s\n", encoded);
  // free(encoded);
  return 0;
}