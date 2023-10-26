#include <../include/Gramma.h>

int main(int argc, char * argv[]){
    if(argc < 2){
        ANALYSIS_MODE = RECUR_ANALY;
    }
    else{
        char * option = argv[1];
        if(strcmp("-h", option) == 0){
            puts("Usage:\tgramma [-option]");
            puts("[-option]:");
            puts("\t-r : Recursive calling procedures enable top-down analysis");
            puts("\t-d : LL(1) parser");
            puts("\t-u : Enable bottom-up analysis");
            exit(0);
        }
        else if(strcmp("-r", option) == 0){
            // 递归调用
        }
        else if(strcmp("-d", option) == 0){
            // LL文法
            ANALYSIS_MODE = LL_ANALY;
        }
        else if(strcmp("-u", option) == 0){
            // LR文法
            ANALYSIS_MODE = LR_ANALY;
        }
        else{
            printf("Invalid Argument %s, use -h to obtain guidance.\n", argv[1]);
            exit(-1);
        }
    }
    switch (ANALYSIS_MODE)
    {
    case RECUR_ANALY:{
        puts("RECUR");
    }break;
    case LL_ANALY:{
        puts("LL");
    }break;
    case LR_ANALY:{
        puts("LR");
    }break;
    default:
        exit(-1);
    }
}