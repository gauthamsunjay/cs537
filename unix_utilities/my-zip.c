#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("my-zip: file1 [file2 ...]\n");
        exit(1);
    }

    int count=1;
    FILE *fp;
    int i, lastchar, presentchar; 
    
    for (i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");

        if (fp == NULL) {
            printf("my-zip: cannot open file\n");
            exit(1);
        }

        if (i == 1)
            lastchar = getc(fp);
        
        while ((presentchar = getc(fp)) != EOF) {
            if (presentchar == lastchar) {
                count++;
            }
            
            else {
                fwrite(&count, sizeof(int), 1, stdout);
                fwrite(&lastchar, sizeof(char), 1, stdout);
                lastchar = presentchar;
                count = 1;
            }
        }
        fclose(fp);
    }

    fwrite(&count, sizeof(int), 1, stdout);
    fwrite(&lastchar, sizeof(char), 1, stdout);

    return 0;
}
