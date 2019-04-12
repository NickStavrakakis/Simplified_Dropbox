#include "functions.h"

static volatile int signalPid = -1;
static volatile int quitting = 0;

void get_pid(int sig, siginfo_t *info, void *context){
    signalPid = info->si_pid;
}

void sig_handler(int dummy){
    quitting = 1;
}

int main(int argc, char *argv[]){

    int i, client_id, buffer_size, transfer_status, retry_status, wait_status;
    char log_file_name[DIR_NAME_LENGTH], common_dirname[DIR_NAME_LENGTH], input_dirname[DIR_NAME_LENGTH], mirror_dirname[DIR_NAME_LENGTH], id_dir_name[DIR_NAME_LENGTH];
    char path_name[PATH_NAME_LENGTH], tmp_str[100], *receiver_id_str;
    char to_check[DIR_NAME_LENGTH], to_delete[DIR_NAME_LENGTH];
    FILE *log_file, *process_file;
    DIR *common_dir, *input_dir, *mirror_dir;
    pid_t sender_pid, receiver_pid, deleter_pid;
    struct dirent *ent;
    struct sigaction sa;

    //./mirror_client -n id -c common_dir -i input_dir -m mirror_dir -b buffer_size -l log_file
    //getting the pramaters
    if(argc!=13){
        printf("Error: Wrong Arguments\n");
        return 0;
    }
    else{
        for(i=1; i<=12; i+=2){

            if(strcmp(argv[i], "-n")==0){
                client_id = atoi(argv[i+1]);
            }
            else if(strcmp(argv[i], "-c")==0){
                strcpy(common_dirname, argv[i+1]);
                if( (common_dir = opendir(common_dirname)) == NULL){
                    mkdir(common_dirname, 0777);
                }
            }
            else if(strcmp(argv[i], "-i")==0){
                strcpy(input_dirname, argv[i+1]);
                if( (input_dir = opendir(input_dirname)) == NULL){
                    fprintf(stderr, "%s does not exist\n", input_dirname);
                    return -1;
                }
                closedir(input_dir);
            }
            else if(strcmp(argv[i], "-m")==0){
                strcpy(mirror_dirname, argv[i+1]);
                if( (mirror_dir = opendir(mirror_dirname)) != NULL){
                    fprintf(stderr, "%s already exists\n", mirror_dirname);
                    closedir(mirror_dir);
                    return -1;
                }
                else{
                    mkdir(mirror_dirname, 0777);
                }
            }
            else if(strcmp(argv[i], "-b")==0){
                buffer_size = atoi(argv[i+1]);
            }
            else if(strcmp(argv[i], "-l")==0){
                strcpy(log_file_name, argv[i+1]);
            }
            else{
                printf("Error: Wrong Arguments\n");
                return 0;
            }
        }
    }

    log_file = fopen(log_file_name, "w+");
    sprintf(tmp_str, "%d\n", client_id);
    fprintf(log_file, "%d, 0, 0, 0, 0, 0", client_id);
    fclose(log_file);

    sprintf(id_dir_name, "%d.id", client_id);
    sprintf(path_name, "%s/%s", common_dirname, id_dir_name);
    if( fopen(path_name, "r") != NULL){
        fprintf(stderr, "%s already exists\n", path_name);
        return -1;
    }
    process_file = fopen(path_name, "w");
    fprintf(process_file, "%d\n", getpid());
    fclose(process_file);

    Executed *executed_root = NULL;
    while((common_dir = opendir(common_dirname)) != NULL){
        printf("\nChecking for updates\n");
        //check for deleted folders

        //checking its self first
        sprintf(to_check, "%s/%d.id", common_dirname, client_id);
        if(access(to_check, F_OK)==-1){
            printf("Warning: Signature file removed from %s. Quitting.\n", common_dirname);
            log_update(log_file_name, 0, 6);
            closedir(common_dir);
            exit(0);
        }
        Executed *current_node = executed_root;
        while(current_node != NULL){
            sprintf(to_check, "%s/%d.id", common_dirname, current_node->receiver_id);
            if(access(to_check, F_OK)==-1){
                deleter_pid = fork();
                if(deleter_pid == -1){
                    perror("Error: Failed to fork");
                    exit(1);
                }
                //CREATING THE DELETER CHILD
                if(deleter_pid == 0){ //we are at the child process
                    sprintf(to_delete, "%s/%d.id", mirror_dirname, current_node->receiver_id);
                    sprintf(receiver_id_str, "%d", current_node->receiver_id);
                    execlp("./deleter", "deleter",  "-d", to_delete, "-r", receiver_id_str, (char*)NULL);
                    printf("Error: deleter execlp execution\n");
                    exit(1);
                }
                tmp_str[0]='\0';
                sprintf(tmp_str, "rm -f %s/id%d_to_id%d.fifo", common_dirname, client_id, current_node->receiver_id);
                system(tmp_str);
                Executed_Remove(&executed_root, current_node->receiver_id);

            }
            current_node = current_node->next;
        }

        //check for new folders
        while((ent = readdir(common_dir)) != NULL){
            //checking for SIGINT / SIGQUIT SIGNALS
            signal(SIGINT, sig_handler);
            signal(SIGQUIT, sig_handler);
            if(quitting){
                //deleting the mirror_dir
                if(MyDelete(mirror_dirname)!=-1){
                    printf("Deleting %s\n", mirror_dirname);
                }
                else{
                    printf("Error: Unable to delete %s\n", mirror_dirname);
                    exit(1);
                }
                //deleting the common_dir/client_id.id
                sprintf(to_delete, "%s/%d.id", common_dirname, client_id);
                if(MyDelete(to_delete)!=-1){
                    printf("Deleting %s\n", to_delete);
                }
                else{
                    printf("Error: Unable to delete %s\n", to_delete);
                    exit(1);
                }
                tmp_str[0]='\0';
                sprintf(tmp_str, "rm -f %s/id%d_to_*.fifo", common_dirname, client_id);
                system(tmp_str);
                log_update(log_file_name, 0, 6);
                closedir(common_dir);
                exit(0);
            }

            //checking for SIGUSR1 signal
            sa.sa_flags = SA_SIGINFO;
            sa.sa_sigaction = get_pid;
            sigaction(SIGUSR1, &sa, NULL);
            transfer_status = 0;
            retry_status = 0;
            if(signalPid != -1){
                Executed *broke_node = Executed_Get(&executed_root, signalPid, 1);
                if(broke_node == NULL){
                    printf("Error: Unable to track SIGUSR1 sender.\n");
                    exit(1);
                }
                if(broke_node->retry_times>3){
                    printf("Warning: Tried many times with no success. Exiting.\n");
                    exit(0);
                }
                sprintf(to_delete, "%s/%d.id", mirror_dirname, broke_node->receiver_id);
                if(MyDelete(to_delete)!=-1){
                    printf("Warning: The File Transfer between client %d and %s caused an error. Trying again. (%d)\n", client_id, receiver_id_str, broke_node->retry_times);
                    retry_status = 1;
                    transfer_status = 1;
                    sprintf(receiver_id_str, "%d", broke_node->receiver_id);
                }
                else{
                    printf("Error: Unable to delete the mis-transfered data.\n");
                    exit(1);
                }
                signalPid = -1;
            }

            //finding a folder that has not transfered yet
            if(retry_status == 0){
                if((strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) && (strcmp(ent->d_name, id_dir_name)!=0) && (strstr(ent->d_name, ".fifo")==NULL)){
                    if( (Executed_Search(executed_root, ent->d_name) == 0)){
                        receiver_id_str = strtok(ent->d_name, ".id");
                        transfer_status = 1;
                    }
                }
            }

            //running the sender and receiver
            if(transfer_status){
                sender_pid = fork();
                if(sender_pid == -1){
                    perror("Error: Failed to fork");
                    exit(1);
                }
                //CREATING THE SENDER CHILD
                if(sender_pid == 0){ //we are at the child process
                    char str_id[50], str_buffer_size[50];
                    sprintf(str_id, "%d", client_id);
                    sprintf(str_buffer_size, "%d", buffer_size);
                    execlp("./sender", "sender",  "-n", str_id, "-c", common_dirname, "-d", input_dirname, "-r", receiver_id_str, "-b", str_buffer_size, "-l", log_file_name, (char*)NULL);
                    printf("Error: sender execlp execution\n");
                    exit(0);
                }
                else{ //we are at the parent process
                    //CREATING THE RECEIVER CHILD
                    receiver_pid = fork();
                    if(receiver_pid == -1){
                        perror("Error: Failed to fork");
                        exit(1);
                    }
                    if(receiver_pid == 0){ //we are at the child process
                        char str_id[50], str_buffer_size[50];
                        sprintf(str_id, "%d", client_id);
                        sprintf(str_buffer_size, "%d", buffer_size);
                        execlp("./receiver", "receiver",  "-n", str_id, "-c", common_dirname, "-m", mirror_dirname, "-s", receiver_id_str, "-b", str_buffer_size, "-l", log_file_name, (char*)NULL);
                        printf("Error: receiver execlp execution\n");
                        exit(0);
                    }
                    else{ //we are the parent process
                        if(retry_status == 1)
                            Executed_Update(&executed_root, atoi(receiver_id_str), receiver_pid);
                        else{
                            Executed_Add(&executed_root, atoi(receiver_id_str), sender_pid, receiver_pid);
                        }
                        waitpid(sender_pid, &wait_status, 0);
                        //sender ended and we are waiting for the receive to finish
                        if(waitpid(receiver_pid, &wait_status, 0) != -1){
                            if(WIFEXITED(wait_status)){
                                int returned = WEXITSTATUS(wait_status);
                                if(returned == 0){
                                    printf("The File Transfer between client %d and %s has terminated successfully. \n", client_id, receiver_id_str);
                                }
                            }
                        }
                    }
                }
            }
        }

        //checking for updates every 10 seconds
        sleep(10);
    }
    perror ("Error: common_dir");
    return 1;
}
