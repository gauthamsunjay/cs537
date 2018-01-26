#include <stdio.h>
#include <stdlib.h>

#define BUFFER (256)

int main(int argc, char* argv[]) {
    FILE *fp;
    char buffer[BUFFER];

    for (int i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("my-cat: cannot open file\n");
            exit(1);
        }
        
        while (fgets(buffer, BUFFER, fp)) {
            printf("%s", buffer);
        }

        fclose(fp);
    }

    return 0;
}
