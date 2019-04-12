#include "functions.h"

//./sender -n sender_id -c common_dirname -d input_dirname -r receiver_id -b buffer_size -l log_file
int main(int argc, char *argv[]){

    char fifoAB[100], common_dirname[DIR_NAME_LENGTH], input_dirname[DIR_NAME_LENGTH], log_file_name[DIR_NAME_LENGTH];
    int i, pipeAB_fd, sender_id, receiver_id, buffer_size;

    //getting the pramaters
    if(argc!=13){
        printf("Error: Wrong Arguments\n");
        kill(getppid(), SIGUSR1);
    }
    else{
        for(i=1; i<=12; i+=2){
            if(strcmp(argv[i], "-n")==0){
                sender_id = atoi(argv[i+1]);
            }
            else if(strcmp(argv[i], "-c")==0){
                strcpy(common_dirname, argv[i+1]);
            }
            else if(strcmp(argv[i], "-d")==0){
                strcpy(input_dirname, argv[i+1]);
            }
            else if(strcmp(argv[i], "-r")==0){
                receiver_id = atoi(argv[i+1]);
            }
            else if(strcmp(argv[i], "-b")==0){
                buffer_size = atoi(argv[i+1]);
            }
            else if(strcmp(argv[i], "-l")==0){
                strcpy(log_file_name, argv[i+1]);
            }
            else{
                printf("Error: Wrong Arguments\n");
                kill(getppid(), SIGUSR1);
            }
        }
    }
    sprintf(fifoAB, "%s/id%d_to_id%d.fifo", common_dirname, sender_id, receiver_id);

    if(mkfifo(fifoAB, 0666) == -1){
        if(errno != EEXIST){
            perror("Error: mkfifo");
            unlink(fifoAB);
            kill(getppid(), SIGUSR1);
        }
    }

    //writing at the pipe
    PipeWriter(input_dirname, fifoAB, buffer_size, log_file_name);

    // inserting the write end flag
    if((pipeAB_fd = open(fifoAB, O_WRONLY))<0){
        perror("Error: fifoAB");
        unlink(fifoAB);
        kill(getppid(), SIGUSR1);
    }
    write(pipeAB_fd, "00", 2);
    log_update(log_file_name, 2, 2);
    close(pipeAB_fd);

    return 0;
}
