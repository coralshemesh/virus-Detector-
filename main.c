#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link link;
struct link {
    link *nextVirus;
    virus *vir;
};

//1a
virus* readVirus(FILE* f);
void printVirus(virus* virus, FILE* output);

//1b
void list_print(link *virus_list, FILE*);
link* list_append(link* virus_list, virus* data);
void list_free(link *virus_list);
link* load();
void printSeg(link *virus_list, FILE* f);

//1c
void detect_virus(char *buffer, unsigned int size, link *virus_list);

//2b
void kill_virus(char *fileName, int signatureOffset, int signatureSize);

int main(int argc, char **argv) {
    //1b
    char *menu[] = {"Load signatures","Print signatures","Detect viruses","Fix file" ,"Quit",NULL};
    int menuSize=0;
    while(menu[menuSize] != NULL) menuSize++;
    unsigned int minSize=0;
    link *list=NULL;
    do{
        char input1[1];
        for (int i = 0; i < menuSize; i++)
            printf("%d) %s \n",i+1, menu[i]);
        fgets(input1,3,stdin);
        int ch = atoi(input1);
        if (ch == 1) list = load();
        else if (ch == 2) {
            printf( "Enter your output file name: ");
            char input[20];
            char *fileName=NULL;
            fgets(input, 21, stdin);
            sscanf(input," %m[^\n]", &fileName);
            FILE *output = fopen(fileName,"w");
	    if(output == NULL) printf("there is no such file\n");
            else{
	    printSeg(list, output);
            fclose(output);
            free(fileName);
	    }
        }
        else if (ch == 3){
            if(argc == 2){
                FILE *fromArgv = fopen(argv[1],"r");
	    	if(fromArgv == NULL) printf("there is no such file\n");
		else{
                fseek(fromArgv, 0L, SEEK_END);
                minSize = ftell(fromArgv);
                rewind(fromArgv);
                char buffer[10000];
                if(10000<minSize) minSize = 10000;
                fread(buffer,minSize,1,fromArgv);
                detect_virus(buffer,minSize, list);
                fclose(fromArgv);
		}
            }
        }
        else if(ch ==4) {
            printf("Please enter the the starting byte location: ");
            char startByte[10];
            char sigS[10];
            fgets(startByte,20,stdin);
            int startB = atoi(startByte);
            printf("Please enter the sigSize of the virus: ");
            fgets(sigS,20,stdin);
            int vSigSize = atoi(sigS);
            kill_virus(argv[1],startB,vSigSize);
        }
        else if(ch == 5){
            list_free(list);
            exit(0);
        }
    } while (1);
}

//**********************1a********************************
virus* readVirus(FILE* file){
        virus *myVirus = (struct virus*)calloc(1,sizeof(struct virus));
        fread(&myVirus->SigSize, 18, 1, file);
        myVirus->sig = (unsigned char*)calloc(1, myVirus->SigSize);
        fread(myVirus->sig,myVirus->SigSize,1,file );
        return myVirus;
}

void printVirus(virus* virus, FILE* output){
    fprintf(output,"Virus name: %s\n",virus->virusName);
    fprintf(output, "Virus size: %d\nsignature:\n",  virus->SigSize );
    for (int i =0; i < virus->SigSize; i++)
       fprintf(output,"%02X ", virus->sig[i]);
    fprintf(output, "\n\n");
}

//*************************1b************************

void list_print(link *virus_list, FILE* f){
    if(virus_list != NULL){
        link* curr = virus_list;
        while(curr->nextVirus != NULL){
            printVirus(curr->vir, f);
            curr = curr->nextVirus;
        }
        printVirus(curr->vir,f);
    }
}

link* list_append(link* virus_list, virus* data){
      //if we want it first
         link* newLink = malloc(sizeof(link));
         newLink->vir = data;
         newLink->nextVirus=virus_list;
          return newLink;

    //if we want it last
//    if(virus_list == NULL){
//        virus_list = malloc(sizeof(link));
//        virus_list->nextVirus=NULL;
//        virus_list->vir = data;
//    }
//    else{
//        link *prev = virus_list;
//        while (prev->nextVirus != NULL){
//            prev = prev->nextVirus;
//        }
//        prev->nextVirus = malloc(sizeof(link));
//        prev->nextVirus->vir = data;
//        prev->nextVirus->nextVirus=NULL;
//    }
//    return virus_list;
}

void list_free(link *virus_list){
    link *curr;
    while (virus_list != NULL){
        curr = virus_list;
        free(curr->vir->sig);
        free(curr->vir);
        virus_list = virus_list->nextVirus;
        free(curr);
    }
}

link* load(){
    printf("Please enter a file name: ");
    char* inputName = malloc(20);
    fgets(inputName,21,stdin);
    strtok(inputName,"\n");
    FILE* input = fopen(inputName, "r");
    if(input == NULL) printf("there is no such file\n");
    else{
    link *head =NULL;
    if (input != NULL) {
        while (1) {
            virus *myVir = readVirus(input);
            if (feof(input)){
                free(myVir->sig);
                free(myVir);
                break;
            }
            head = list_append(head, myVir);
        }
    }
    free(inputName);
    fclose(input);
    return head;
    }
}

void printSeg(link *virus_list, FILE* f){
    if(virus_list != NULL) list_print(virus_list, f);
}

//********************1c*************************

void detect_virus(char *buffer, unsigned int size, link *virus_list){
    if(virus_list !=NULL){
        link* curr = virus_list;
        while (curr->nextVirus !=NULL){
            for(int i =0; i <= size-(curr->vir->SigSize); i++){
                if(memcmp((buffer+i), curr->vir->sig, curr->vir->SigSize)==0){
                    printf("starting byte location in the suspected file: %d\n",i);
                    printf("virus name: %s\n",curr->vir->virusName);
                    printf("size of virus signature: %d\n", curr->vir->SigSize);
                }
            }
            curr=curr->nextVirus;
        }
        for(int i =0; i <= size-(curr->vir->SigSize); i++){
            if(memcmp((buffer+i), curr->vir->sig, curr->vir->SigSize)==0){
                printf("starting byte location in the suspected file: %d\n",i);
                printf("virus name: %s\n",curr->vir->virusName);
                printf("size of virus signature: %d\n", curr->vir->SigSize);
            }
        }
    }
}

//***************************2b****************************

void kill_virus(char *fileName, int signatureOffset, int signatureSize){
    FILE* f = fopen(fileName, "r+");
    char arr[signatureSize];
    for(int i = 0; i < signatureSize; i++) arr[i] = 0x90;
    fseek(f, signatureOffset, SEEK_SET);
    fwrite(arr, 1, signatureSize,f);
    fclose(f);
}
