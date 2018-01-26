#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_present(char* search_string, char* line) {
   if (strstr(line, search_string) == NULL)
       return 0;
   return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("my-grep: searchterm [file ...]\n");
        exit(1);
    }

    char* search_string = argv[1];
    char* line = NULL;
    size_t linelen;

    if (argc == 2) {     
        while (getline(&line, &linelen, stdin) > 0) {
            if (is_present(search_string, line))
                printf("%s", line);    
        }
    }
    
    else {
        FILE *fp;
        for (int i = 2; i < argc; i++) {
            fp = fopen(argv[i], "r");
            if (fp == NULL) {
                printf("my-grep: cannot open file\n");
                exit(1);
            }

            while (getline(&line, &linelen, fp) > 0) {
                if (is_present(search_string, line))
                    printf("%s", line);
            }

            fclose(fp);
        }
    }

    return 0;
}
