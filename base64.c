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
  // 8 * n / 6 -> 4 * n / 3 gives unpadded length
  // 8 ASCII bits * input_length / 6 base64 bits
  // And round up to the nearest multiple of 4 for padding,
  // and as 4 is a power of 2 can use bitwise logical operations.
  // ((4 * n / 3) + 3) & ~3
  int size = ((4 * strlen(input) / 3) + 3) & ~3;
  char* output = malloc(size);
  unsigned char bits[size * 8];

  // convert input to bits
  int k = 0;
  for (int i = 0; i < strlen(input); i++) {
    for (int j = 7; j >= 0; j--) {
      bits[k++] = (((unsigned char)input[i] >> j) & 1);
    }
  }
  for (int i = strlen(input); i < size; i++) {
    bits[k++] = 0;
  }

  unsigned char subbits[6] = {0};
  int bits_len = strlen(input) * 8;
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

int main() {
  char* to_encode = "Hello World";
  printf("Encoding string: %s\n", to_encode);
  char* encoded = base64_encode(to_encode);
  printf("Encoded string: %s\n", encoded);
  free(encoded);
  return 0;
}