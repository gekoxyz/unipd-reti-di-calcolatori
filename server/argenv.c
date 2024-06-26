#include<stdio.h>
int main(int c, char * argv[], char *env[]){

int i;
for(i=0;i<c;i++)
	printf("argv[%d]=%s\n",i,argv[i]);

for(i=0; env[i]; i++)
	printf("env[%d]=%s\n",i,env[i]);
}


