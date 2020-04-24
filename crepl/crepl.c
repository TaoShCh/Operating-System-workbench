#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#define TMP_DIR_NAME "tmpdir"
#define MAX_STR 1000

#define BOLD_RED_FONT "\033[1;31m"
#define BRIGHT_RED_FONT "\033[0;91m"
#define GREEN_FONT "\033[0;92m"
#define BLUE_FONT "\033[0;36m"
#define RESET "\033[0m"
#define BOLD_FONT "\033[1m"

#if defined(__i386__)
  #define GCC_ENV_OPTION "-m32"
#elif defined(__x86_64__)
  #define GCC_ENV_OPTION "-m64"
#endif

int get_input();
int is_func(char* str);
void *define_and_load();
int evaluate_expr();
void compile_err();
void Helloworld();
void rm_tmpdir();

char input[MAX_STR];
int func_id=0,expr_wrap_id=0;

int get_input(){
    printf(BLUE_FONT">>> "RESET);
    if(fgets(input,MAX_STR,stdin)==NULL) return 0;
    return 1;
}

int is_func(char *str){
    if(strncmp(str,"int",3)==0){
        return 1;
    }
    else return 0;
}

void *define_and_load(){
    char cwd[MAX_STR]="\0",filename[MAX_STR]="\0",libname[MAX_STR]="\0";
    getcwd(cwd,MAX_STR);
    sprintf(filename,"%s/%s/tmp%d.c",cwd,TMP_DIR_NAME,func_id);
    sprintf(libname,"%s/%s/temp%d.so",cwd,TMP_DIR_NAME,func_id);
    //create the function file
    FILE *fp=fopen(filename,"w");
    fprintf(fp,"%s",input);
    assert(fclose(fp)==0);
    //compile
    int val=fork();
    if(!val){
        //child process, redirect the output
        int fd=open("/dev/null",O_WRONLY);
        dup2(fd,STDERR_FILENO);
        execlp("gcc","gcc","-shared","-fPIC",
        "-Wno-implicit-function-declaration",GCC_ENV_OPTION,
        "-o",libname,filename,(char *)NULL);
        assert(0);
    }
    else{
        wait(NULL);
    }
    assert(remove(filename)==0);
    func_id++;
    return dlopen(libname, RTLD_LAZY|RTLD_GLOBAL);
}

int evaluate_expr(int *succ_flag){
    //wrap the func
    char func_name[MAX_STR]="\0";
    sprintf(func_name,"_expr_wrap_%d",func_id);
    char func_body[MAX_STR]="\0";
    sprintf(func_body,"int %s(){return (%s);}",func_name,input);
    strcpy(input,func_body);
    //load the expr_func
    void *handle=define_and_load();
    if(handle!=NULL){
        int (*func)();
        func=dlsym(handle,func_name);
        if(func==NULL){
            *succ_flag=0;
            return 0;
        }
        else {
            *succ_flag=1;
            return func();
        }
    }
    else{
        *succ_flag=0;
        return 0;
    }
}

void compile_err(){
    printf(BRIGHT_RED_FONT"   Compile Error!\n");
}

void Helloworld(){
    printf(BOLD_RED_FONT"   Welcome to my CREPL!"RESET"\n");
    printf(BOLD_RED_FONT"   Press Ctrl+D to exit."RESET"\n");
}

void rm_tmpdir(){
    char cmd[MAX_STR]="rm -rf ";
    strcat(cmd,TMP_DIR_NAME);
    assert(system(cmd)!=-1);
}

int main(int argc, char *argv[]) {
    Helloworld();
    if(mkdir(TMP_DIR_NAME,S_IRWXU)==-1&&errno==EEXIST){
        rm_tmpdir();
        assert(mkdir(TMP_DIR_NAME,S_IRWXU)!=-1);
    }
    int flag;
    while(get_input()){
        if(is_func(input)){
            if(define_and_load()!=NULL){
                printf(GREEN_FONT"   Added"RESET": %s",input);
            }
            else compile_err();
        }
        else{
            int val=evaluate_expr(&flag);
            if(flag){
                printf("   Ans = %d.\n",val);
            }
            else compile_err();
        }
        memset(input,0,sizeof(input));
    }
    printf("\n");
    rm_tmpdir();
    return 0;
}
