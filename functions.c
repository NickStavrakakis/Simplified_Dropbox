#include "functions.h"

void log_update(char log_file_name[DIR_NAME_LENGTH], int value, int c){

    Statistics mystats;
    FILE *log_file;

    if((log_file = fopen(log_file_name,"r")) == NULL){
        printf("Error: %s does not exist\n", log_file_name);
        exit(1);
    }
    fscanf(log_file, "%d ,%d, %d, %d, %d, %d \n", &mystats.client_id, &mystats.s_bytes, &mystats.r_bytes,
                                                  &mystats.s_files, &mystats.r_files, &mystats.deleted_flag);
    fclose(log_file);

    log_file = fopen(log_file_name, "w+");
    switch(c){
        case 2:
            fprintf(log_file, "%d, %d, %d, %d, %d, %d \n", mystats.client_id, mystats.s_bytes+value, mystats.r_bytes,
                                                           mystats.s_files, mystats.r_files, mystats.deleted_flag);
            break;
        case 3:
            fprintf(log_file, "%d, %d, %d, %d, %d, %d \n", mystats.client_id, mystats.s_bytes, mystats.r_bytes+value,
                                                           mystats.s_files, mystats.r_files, mystats.deleted_flag);
            break;
        case 4:
            fprintf(log_file, "%d, %d, %d, %d, %d, %d \n", mystats.client_id, mystats.s_bytes, mystats.r_bytes,
                                                           ++mystats.s_files, mystats.r_files, mystats.deleted_flag);
            break;
        case 5:
            fprintf(log_file, "%d, %d, %d, %d, %d, %d \n", mystats.client_id, mystats.s_bytes, mystats.r_bytes,
                                                           mystats.s_files, ++mystats.r_files, mystats.deleted_flag);
            break;
        case 6:
            fprintf(log_file, "%d, %d, %d, %d, %d, %d \n", mystats.client_id, mystats.s_bytes, mystats.r_bytes,
                                                           mystats.s_files, mystats.r_files, ++mystats.deleted_flag);
            break;
    }
    fclose(log_file);
}


int MyDelete(char *folder_name){
    char command[DIR_NAME_LENGTH+7];
    sprintf(command, "rm -rf %s", folder_name);
    return system(command);
}

char *RemoveStr(char *str, const char *sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

void PipeWriter(char input_dirname[DIR_NAME_LENGTH], char fifoAB[50], int buffer_size, char log_file_name[DIR_NAME_LENGTH]){

    int remaining_file_length, pipeAB_fd, received_f_length;
    char current_filename[DIR_NAME_LENGTH], path_name[PATH_NAME_LENGTH], received_f_name_length[SHRT_MAX];
    FILE *current_fd;
    DIR *input_dir;
    struct dirent *ent;

    input_dir = opendir(input_dirname);
    while((ent = readdir(input_dir)) != NULL){
        strcpy(current_filename, ent->d_name);
        if( (strcmp(current_filename, ".")!=0) && (strcmp(current_filename, "..")!=0) && (strcmp(current_filename, input_dirname))){
            sprintf(path_name, "%s/%s", input_dirname, current_filename);
            sprintf(current_filename, "%s", strstr(RemoveStr(path_name, "./"), "/"));

            if(ent->d_type == DT_DIR){
                //do this again but instead of input_dir do for ent->d_type
                if((pipeAB_fd = open(fifoAB, O_WRONLY))<0){
                    perror("Error: fifoAB");
                    exit(1);
                }
                // inserting the size of the file name
                sprintf(current_filename, "%s/", current_filename); //adding a / at the end to know if it is a bracket
                sprintf(received_f_name_length, "%ld", strlen(current_filename));
                write(pipeAB_fd, received_f_name_length, 2);

                log_update(log_file_name, 2, 2);

                // inserting the name of the file
                write(pipeAB_fd, current_filename, strlen(current_filename));
                log_update(log_file_name, strlen(current_filename), 2);

                close(pipeAB_fd);
                PipeWriter(path_name, fifoAB, buffer_size, log_file_name);
            }
            else if(ent->d_type == DT_REG){
                if((pipeAB_fd = open(fifoAB, O_WRONLY))<0){
                    perror("Error: fifoAB");
                    exit(1);
                }
                log_update(log_file_name, 0, 4);
                // inserting the size of the filecurrent_filename name
                sprintf(received_f_name_length, "%ld", strlen(current_filename));
                write(pipeAB_fd, received_f_name_length, 2);
                log_update(log_file_name, 2, 2);

                // inserting the name of the file
                write(pipeAB_fd, current_filename, strlen(current_filename));
                log_update(log_file_name, strlen(current_filename), 2);

                // inserting the size of the file
                received_f_length = count_chars(path_name);
                write(pipeAB_fd, &received_f_length, 4);
                log_update(log_file_name, 4, 2);

                // inserting the file
                current_fd = fopen(path_name, "r");
                remaining_file_length = received_f_length;
                if(current_fd){
                    char *buffer;
                    while(remaining_file_length){
                        if(remaining_file_length >=buffer_size){
                            buffer = malloc(buffer_size+1);
                            memset(buffer, '\0', buffer_size+1);
                            fread(buffer, buffer_size, 1, current_fd);
                            write(pipeAB_fd, buffer, buffer_size);
                            log_update(log_file_name, buffer_size, 2);
                            remaining_file_length -= buffer_size;
                        }
                        else{
                            buffer = realloc(buffer, remaining_file_length+1);
                            memset(buffer, '\0', remaining_file_length+1);
                            fread(buffer, remaining_file_length, 1, current_fd);
                            write(pipeAB_fd, buffer, remaining_file_length);
                            log_update(log_file_name, remaining_file_length, 2);
                            remaining_file_length = 0;
                        }
                    }
                    free(buffer);
                }
                fclose(current_fd);
                close(pipeAB_fd);
            }
            else{
                printf("Warning: Skipping %s because is neither a file or a folder.\n", current_filename);
            }
        }
    }
    return;
}

Executed* Executed_Get(Executed** root, int receiver_pid, int flag){

    Executed* current = *root;
    while(current != NULL){
        if(current->receiver_pid == receiver_pid){
            if(flag==1)
                current->retry_times++;
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int Executed_Search(Executed* root, char *id_dir_name){

    Executed* current = root;
    int receiver_id = atoi(strtok(id_dir_name, ".id"));
    while(current != NULL){
        if(current->receiver_id == receiver_id){
            return 1;
        }
        current = current->next;
    }
    return 0;

}

int Executed_Update(Executed** root, int receiver_id, int receiver_pid){

    Executed* current = *root;
    while(current != NULL){
        if(current->receiver_id == receiver_id){
            current->receiver_pid = receiver_pid;
            return 1;
        }
        current = current->next;
    }
    return 0;

}

void Executed_Add(Executed** root, int receiver_id, int sender_pid, int receiver_pid){

    Executed* new = (Executed*)malloc(sizeof(Executed));
    Executed* last = *root;

    new->receiver_id = receiver_id;
    new->sender_pid = sender_pid;
    new->receiver_pid = receiver_pid;
    new->retry_times = 0;
    new->next = NULL;


    if(*root == NULL){
        *root = new;
        return;
    }

    while(last->next != NULL){
        last = last->next;
    }

    last->next = new;
    return;
}

void Executed_Remove(Executed** root, int receiver_id){

    Executed* tmp = *root, *prev;

    if((tmp != NULL) && (tmp->receiver_id == receiver_id)){
        *root = tmp->next;
        free(tmp);
        return;
    }

    while((tmp != NULL) && (tmp->receiver_id != receiver_id)){
        prev = tmp;
        tmp = tmp->next;
    }

    prev->next = tmp->next;

    free(tmp);

    return;
}



int count_chars(char file_name[DIR_NAME_LENGTH]){

    int total = 0;
    char next_char;
    FILE *fd;
    fd = fopen(file_name, "r");

    while((next_char = getc(fd))!=EOF){
        total++;
    }
    fclose(fd);

    return total;
}
