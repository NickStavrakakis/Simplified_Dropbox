#include "functions.h"

// ./deleter -d dir_to_delete -r id_of_receiver
int main(int argc, char *argv[]){

    char to_delete[DIR_NAME_LENGTH];
    int i, receiver_id;

    //getting the pramaters
    if(argc!=5){
        printf("Error: Wrong Arguments\n");
        return 1;
    }
    else{
        for(i=1; i<=4; i+=2){
            if(strcmp(argv[i], "-d")==0){
                strcpy(to_delete, argv[i+1]);
            }
            else if(strcmp(argv[i], "-r")==0){
                receiver_id = atoi(argv[i+1]);
            }
            else{
                printf("Error: Wrong Arguments\n");
                return 1;
            }
        }
    }

    if(MyDelete(to_delete)!=-1){
        printf("Client %d has been disappeared. Deleting its files.\n", receiver_id);
    }
    else{
        printf("Error: Unable to delete its data\n");
        return 1;
    }

    return 0;
}
