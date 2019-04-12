#define DIR_NAME_LENGTH 100
#define PATH_NAME_LENGTH 500
#define TEXT_LINE_LENGTH 300

typedef struct Executed{
    int receiver_id;
    int sender_pid;
    int receiver_pid;
    int retry_times;
    struct Executed *next;
} Executed;

typedef struct Statistics{
    int client_id;
    int s_bytes;
    int r_bytes;
    int s_files;
    int r_files;
    int deleted_flag;
} Statistics;
