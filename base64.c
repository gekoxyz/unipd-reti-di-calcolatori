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

  unsigned char subbits[0];
  int start = 0;

  for (int i = start; i <= start + 6; i++) {
    subbits[i - start] = bits[i];
  }

  for (int i = 0; i < 7; i++) {
    printf("%d", subbits[i]);
  }
  printf("\n");

  // int decimal_values[(strlen(input)*8)/6 + 1]; // array to store decimal values
  // int length = sizeof(decimal_values) / sizeof(decimal_values[0]);
  
  // printf("result of the conversion\n");
  // for (int i = 0; i < length; i++) {
  //   printf("%d -> %d\n", i, decimal_values[i]);
  // }

  return "output";
}

int main() {
  char* to_encode = "Hello World";
  printf("Encoding string: %s\n", to_encode);
  char* encoded = base64_encode(to_encode);
  printf("Encoded string: %s\n", encoded);
  return 0;
}