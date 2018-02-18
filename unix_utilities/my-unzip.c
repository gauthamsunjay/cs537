#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("my-unzip: file1 [file2 ...]\n");
        exit(1);
    }

    int i, j, count;
    FILE *fp;
    char ch; 
    
    for (i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("my-unzip: cannot open file\n");
            exit(1);
        }
        
        while (1) {
            if (!fread(&count, sizeof(int), 1, fp))
                break;
            if (!fread(&ch, sizeof(char), 1, fp))
                break;
           
            for (j = 0; j < count; j++) {
                printf("%c", ch);
            }
        }
        
        fclose(fp);     
    }

    return 0;
}
