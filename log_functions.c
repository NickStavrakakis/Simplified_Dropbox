#include "functions.h"

void log_update_sbytes(char log_file_name[DIR_NAME_LENGTH], int s_bytes){

    Statistics mystats;

    FILE *log_file;
    char line[10];
    if((log_file = fopen(log_file_name,"r")) == NULL){
        printf("Error: %s does not exist\n", log_file_name);
        exit(1);
    }
    fscanf(log_file, "%d ,%d, %d, %d, %d, %d \n",   &mystats.client_id,
                                                    &mystats.s_bytes,
                                                    &mystats.r_bytes,
                                                    &mystats.s_files,
                                                    &mystats.r_files,
                                                    &mystats.deleted_flag);
    fclose(log_file);

    log_file = fopen(log_file_name, "w+");
    fprintf(log_file, "%d\n%d\n%d\n%d\n%d\n%d \n",    mystats.client_id,
                                                      mystats.s_bytes+s_bytes,
                                                      mystats.r_bytes,
                                                      mystats.s_files,
                                                      mystats.r_files,
                                                      mystats.deleted_flag);
    fclose(log_file);
}
