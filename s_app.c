#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>



void error() {
    printf("Required: -f <input.txt> | -o <output.txt>\n" );
    exit(2);
}
int main (int argc, char *argv []) 


{
//Declarations
    int inputOpt;
    int f_flag =0, o_flag = 0;

    if (argc<4){
        error();
    }
    
    while ((inputOpt = getopt(argc, argv, "f:o:")) != -1 ){

        switch (inputOpt)
        {
        case 'f':
            printf("f: %s\n", optarg);
            f_flag = 1;
            break;
        case 'o':
            printf("o: %s\n", optarg);
            o_flag = 1;
        default:
            printf("Invalid option: %c\n", inputOpt);
            error();
            break;
        }
    }
//Check if required options are given
   if (!f_flag || !o_flag) {
    printf("Both -f and -o options are required\n");
    error();
}

}
