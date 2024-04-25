#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

char* base64_encode(char* input) {
  unsigned char bits[strlen(input)*8];
  unsigned char substr[7];

  int k = 0;
  printf("\n");
  for (int i = 0; i < strlen(input); i++) {
    for (int j = 7; j >= 0; j--) {
      // printf("%d", (((unsigned char)input[i] >> j) & 1));  // print each bit
      bits[k] = (((unsigned char)input[i] >> j) & 1);
      k++;
    }
  }

  // bits contains now the string converted in bits

  for (int c = 0; c < (strlen(input)*8); c++) {
    printf("%d",bits[c]);
  }
  printf("\n\n");

  // CONVERSION

  printf("starting bits to dec conversion\n");

  // int decimal_values[(strlen(input)*8)/6 + 1]; // array to store decimal values
  // int length = sizeof(decimal_values) / sizeof(decimal_values[0]);
  
  // printf("result of the conversion\n");
  // for (int i = 0; i < length; i++) {
  //   printf("%d -> %d\n", i, decimal_values[i]);
  // }
  
  printf("\n");
  return "output";
}

void convertToDecimal(const unsigned char *bits, int bitsLength, int *decimalValues) {
    int i, j;
    unsigned char mask = 0x3F; // Mask to extract 6 bits (00111111 in binary)

    for (i = 0, j = 0; i < bitsLength; i += 6, j++) {
        unsigned char chunk = bits[i / 8] >> (i % 8); // Shift to align with the current byte
        chunk &= mask; // Apply mask to extract 6 bits
        decimalValues[j] = chunk; // Store the decimal value
    }
}

int main() {
  char* to_encode = "Hello World";
  printf("Encoding string: %s\n", to_encode);
  char* encoded = base64_encode(to_encode);
  printf("Encoded string: %s\n", encoded);
  return 0;
}