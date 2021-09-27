#include <stdio.h>

FILE *fPoint;
void getToken() {
    while(!feof(fPoint)) {
        
    }
}

int main(int argc, char *argv[]) {
    printf("Hello World\n");
    fPoint = fopen(argv[0], "r");
    if(fPoint==NULL) {
        printf("Error: Wrong Input File!\n"); 
    } else {
        getToken();
    }
}