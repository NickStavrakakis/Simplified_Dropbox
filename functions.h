#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <limits.h>
#include "structs.h"

int MyDelete(char *folder_name);

char *RemoveStr(char *str, const char *sub);

void PipeWriter(char input_dirname[DIR_NAME_LENGTH], char fifoAB[50], int buffer_size, char log_file_name[DIR_NAME_LENGTH]);

Executed* Executed_Get(Executed** root, int receiver_pid, int flag);

int Executed_Search(Executed* root, char *id_dir_name);

int Executed_Update(Executed** root, int receiver_id, int receiver_pid);

void Executed_Add(Executed** root, int receiver_id, int sender_pid, int receiver_pid);

void Executed_Remove(Executed** root, int receiver_id);

int count_chars(char file_name[DIR_NAME_LENGTH]);

void sig_handler(int signo);

void log_update(char log_file_name[DIR_NAME_LENGTH], int value, int c);
