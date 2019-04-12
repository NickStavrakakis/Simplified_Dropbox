#include "functions.h"

//./receiver -n receiver_id -c common_dirname -m mirror_dirname -r sender_id -s receiver_id -b buffer_size -l log_file
int main(int argc, char *argv[]){

    char fifoBA[100], mirror_dirname[DIR_NAME_LENGTH], path_name[PATH_NAME_LENGTH], log_file_name[DIR_NAME_LENGTH], common_dirname[DIR_NAME_LENGTH];
    char received_f_name_length[SHRT_MAX];
    char end_flag[4];
    int i, pipeBA_fd, receiver_id, sender_id, buffer_size, remaining_file_length, received_f_length;
    FILE *current_fd;

    //getting the pramaters
    if(argc!=13){
        printf("Error: Wrong Arguments\n");
        kill(getppid(), SIGUSR1);
    }
    else{
        for(i=1; i<=12; i+=2){
            if(strcmp(argv[i], "-n")==0){
                receiver_id = atoi(argv[i+1]);
            }
            else if(strcmp(argv[i], "-c")==0){
                strcpy(common_dirname, argv[i+1]);
            }
            else if(strcmp(argv[i], "-m")==0){
                strcpy(mirror_dirname, argv[i+1]);
            }
            else if(strcmp(argv[i], "-s")==0){
                sender_id = atoi(argv[i+1]);
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
    sprintf(fifoBA, "%s/id%d_to_id%d.fifo", common_dirname, sender_id, receiver_id);

    while((pipeBA_fd = open(fifoBA, O_RDONLY))<0){}

    //creating the receiver directory inside mirror, if does not exists
    struct stat st = {0};
    sprintf(path_name, "%s/%d.id", mirror_dirname, sender_id);

    if (stat(path_name, &st) == -1) {
        mkdir(path_name, 0700);
    }

    sleep(5);
    while(1){
        memset(received_f_name_length, 0, SHRT_MAX);
        memset(path_name, 0, DIR_NAME_LENGTH);
        char *received_f_name = NULL;

        read(pipeBA_fd, received_f_name_length, 2);
        log_update(log_file_name, 2, 3);
        memset(end_flag, 0, 2);
        if( (strlen(received_f_name_length)==2) && (strcmp(received_f_name_length, "00")==0)){
            unlink(fifoBA);
            close(pipeBA_fd);
            exit(0);
        }

        received_f_name = calloc(1, (atoi(received_f_name_length))*sizeof(char));
        read(pipeBA_fd, received_f_name, atoi(received_f_name_length));
        log_update(log_file_name, atoi(received_f_name_length), 3);
        sprintf(path_name, "%s/%d.id%s", mirror_dirname, sender_id, received_f_name);
        if(received_f_name[atoi(received_f_name_length)-1]=='/'){
            mkdir(path_name, 0777);
        }
        else{
            current_fd = fopen(path_name, "w");
            log_update(log_file_name, 0, 5);
            //
            read(pipeBA_fd, &received_f_length, 4);
            log_update(log_file_name, 4, 3);
            //
            remaining_file_length = received_f_length;
            char *received_f;
            while(remaining_file_length){
                if(remaining_file_length >= buffer_size){
                    received_f = calloc(1, buffer_size); //HEY
                    read(pipeBA_fd, received_f, buffer_size);
                    log_update(log_file_name, buffer_size, 3);
                    remaining_file_length -= buffer_size;
                }
                else{
                    received_f = calloc(1, received_f_length);
                    read(pipeBA_fd, received_f, remaining_file_length);
                    log_update(log_file_name, remaining_file_length, 3);
                    remaining_file_length = 0;
                }
                fputs(received_f, current_fd);
                free(received_f);
                received_f = NULL;
            }
            printf("Just received %s\n", path_name);
            fclose(current_fd);
        }
        free(received_f_name);
        received_f_name = NULL;
    }
    unlink(fifoBA);
    close(pipeBA_fd);
    kill(getppid(), SIGUSR1);
    return 0;
}
