
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>


#define INPUT_SIZE 510
#define CUTTING_WORD " \n"
#define ENDING_WORD "done\n"
#include <errno.h>
char *getcwd(char *buf, size_t size);

void  DisplayPrompt();
char** execFunction(char *input,char **argv,int *sizeOfArray,int *cmdLength);
void garbageCollector(char** argv,int size);


static int *numOfCmd;
static int *cmdLength;
int main() {

    numOfCmd = mmap(NULL, sizeof *numOfCmd, PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    cmdLength = mmap(NULL, sizeof *cmdLength, PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    (*numOfCmd)=0;
    (*cmdLength)=0;
    int sizeOfArray=0;

    char input[INPUT_SIZE]="";//A string array containing the input.
    DisplayPrompt();
    pid_t id; // pid_t use for process identifer
    char **argv;//A string array will containing the program name and command arguments

    while (strcmp(input,ENDING_WORD)!=0)
    {
        if(fgets(input,INPUT_SIZE,stdin)==0)
            printf(" ");
            //do nothing...countine regular

	argv=execFunction(input,argv,&sizeOfArray,cmdLength);
	 if (strcmp("cd",argv[0])==0)
    {
        struct passwd *pwd;
        char* path=argv[1];

        if(path==NULL)
        {
            pwd=getpwuid(getuid());
            path=pwd->pw_dir;
        }
        errno=chdir(path);
        DisplayPrompt();
        if(errno!=0)
            printf("error changing dircatory");
       
    }

	else
	{
        id=fork();
        if (id<0)
        {
            printf("fork faild");
            exit(0);
        }
        if(id==0) {
            (*numOfCmd)++;
            
            execvp(argv[0],argv);
 	    garbageCollector(argv,sizeOfArray);
            if(strcmp(input,ENDING_WORD)!=0)
                exit(1);
        }else {
            wait(&id);
            
                if (strcmp(input, ENDING_WORD) != 0)
                    DisplayPrompt();
                else {
                    printf("Num of cmd: %d\n", *numOfCmd-1);
                    printf("cmd length: %d\n", *cmdLength-4);
                    printf("Bye !\n");
                }
            }

        }
    }
    return 0;
}
void garbageCollector(char** argv,int size)
{
    int i=0;
    for (i = 0; i < size; ++i) {
        free(argv[i]);
    }
    free(argv);
    argv=NULL;
}
char** execFunction(char *input,char **argv,int *sizeOfArray,int *cmdLength)
{
    int i=0,counter=0;
    char inputCopy[INPUT_SIZE];
    strcpy(inputCopy,input);

    char* ptr= strtok(input,CUTTING_WORD);
    while(ptr!=NULL)
    {
        ptr=strtok(NULL,CUTTING_WORD);
        counter++;
    }
    argv = (char**)malloc((counter+1)*(sizeof(char*)));
    if(argv==NULL)
    {
        printf("error allocated");
        exit(0);
    }

    char* ptrCopy= strtok(inputCopy,CUTTING_WORD);
    while(ptrCopy!=NULL)
    {
        if (i==0)
            (*cmdLength)+=strlen(ptrCopy);
        argv[i]=(char*)malloc((sizeof(char)+1)*strlen(ptrCopy));
        if(argv[i]==NULL)
        {
            printf("error allocated");
            for (int j = i-1; j >-1 ; j--) {
                free(argv[j]);
            }
            free(argv);
            exit(0);
        }
        strcpy(argv[i],ptrCopy);
        argv[i][strlen(ptrCopy)]='\0';
        ptrCopy=strtok(NULL,CUTTING_WORD );
        i++;
    }
    argv[counter]=NULL;
    (*sizeOfArray)=counter;
return argv;
   
}

void DisplayPrompt()
{

//-------------------show the path-----------------------------

    long size;
    char *buf;
    char *ptr;

    size = pathconf(".", _PC_PATH_MAX);

    if ((buf = (char *)malloc((size_t)size)) != NULL)
        ptr = getcwd(buf, (size_t)size);


    //----------show the user name root------------------------

    struct passwd *getpwuid(uid_t uid);
    struct passwd *p;
    uid_t uid=0;
    if ((p = getpwuid(uid)) == NULL)
        perror("getpwuid() error");
    else {
        printf("%s@%s>\n", p->pw_name, ptr);
    }
}
