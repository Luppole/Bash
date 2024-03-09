#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <curses.h>

#define MAX_LINE 1024
#define READING_CAPCACITY 100000
#define MAX_SIZE 256
#define MAX_DIR  128

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

//Regular underline text
#define UBLK "\e[4;30m"
#define URED "\e[4;31m"
#define UGRN "\e[4;32m"
#define UYEL "\e[4;33m"
#define UBLU "\e[4;34m"
#define UMAG "\e[4;35m"
#define UCYN "\e[4;36m"
#define UWHT "\e[4;37m"

//Regular background
#define BLKB "\e[40m"
#define REDB "\e[41m"
#define GRNB "\e[42m"
#define YELB "\e[43m"
#define BLUB "\e[44m"
#define MAGB "\e[45m"
#define CYNB "\e[46m"
#define WHTB "\e[47m"

//High intensty background 
#define BLKHB "\e[0;100m"
#define REDHB "\e[0;101m"
#define GRNHB "\e[0;102m"
#define YELHB "\e[0;103m"
#define BLUHB "\e[0;104m"
#define MAGHB "\e[0;105m"
#define CYNHB "\e[0;106m"
#define WHTHB "\e[0;107m"

//High intensty text
#define HBLK "\e[0;90m"
#define HRED "\e[0;91m"
#define HGRN "\e[0;92m"
#define HYEL "\e[0;93m"
#define HBLU "\e[0;94m"
#define HMAG "\e[0;95m"
#define HCYN "\e[0;96m"
#define HWHT "\e[0;97m"

//Bold high intensity text
#define BHBLK "\e[1;90m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHBLU "\e[1;94m"
#define BHMAG "\e[1;95m"
#define BHCYN "\e[1;96m"
#define BHWHT "\e[1;97m"

//Reset
#define reset "\e[0m"
#define CRESET "\e[0m"
#define COLOR_RESET "\e[0m"
                                                                                               
                                                                                               
typedef struct {
    char data[MAX_SIZE][MAX_LINE];
    int top;
} Stack;

void initialize(Stack *stack) {
    stack->top = -1;
}

int is_empty(Stack *stack) {
    return stack->top == -1;
}

int is_full(Stack *stack) {
    return stack->top == MAX_SIZE - 1;
}

void push(Stack *stack, const char *value) {
    if (is_full(stack)) {
        printf("Stack overflow\n");
        exit(EXIT_FAILURE);
    }

    strcpy(stack->data[++stack->top], value);
}

const char *pop(Stack *stack) {
    if (is_empty(stack)) {
        printf("Stack underflow\n");
        exit(EXIT_FAILURE);
    }

    return stack->data[stack->top--];
}

typedef struct {
    char *file_type;
    off_t file_size;
    char *owner;
    char *group;
    char *permissions;
} file_info_t;

file_info_t file_info(const char *file_name) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error getting current working directory");
        exit(1);
    }

    char file_path[1024];
    snprintf(file_path, sizeof(file_path), "%s/%s", cwd, file_name);

    struct stat file_stat;
    if (stat(file_path, &file_stat) < 0) {
        perror("Error getting file status");
        exit(1);
    }

    file_info_t file_info;
    memset(&file_info, 0, sizeof(file_info_t));

    if (S_ISREG(file_stat.st_mode)) {
        file_info.file_type = "regular file";
    } 

    else if(strstr(file_name, '.') == NULL) {
    	file_info.file_type = "executable binary file";
    }

    else if (S_ISDIR(file_stat.st_mode))
    {
        file_info.file_type = "directory";
    } 

    else if (S_ISCHR(file_stat.st_mode))
    {
        file_info.file_type = "character device";
    }

    else if (S_ISBLK(file_stat.st_mode)) 
    {
        file_info.file_type = "block device";
    } 

    else if (S_ISFIFO(file_stat.st_mode)) 
    {
        file_info.file_type = "FIFO";
    } 

    else if (S_ISLNK(file_stat.st_mode)) 
    {
        file_info.file_type = "symbolic link";
    } 

    else if (S_ISSOCK(file_stat.st_mode)) 
    {
        file_info.file_type = "socket";
    } 

    else 
    {
        file_info.file_type = "unknown file type";
    }

    file_info.file_size = file_stat.st_size;

    struct passwd *pwd = getpwuid(file_stat.st_uid);
    if (pwd != NULL) {
        file_info.owner = strdup(pwd->pw_name);
    }

    struct group *grp = getgrgid(file_stat.st_gid);
    if (grp != NULL) {
        file_info.group = strdup(grp->gr_name);
    }

    return file_info;
}

int cd(const char *directory) {
    if (chdir(directory) != 0) {
        perror("chdir");
        return 1;  // Return an error status
    }

    // Print the current working directory after changing
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
        return 0;  // Return success status
    } else {
        perror("getcwd");
        return 1;  // Return an error status
    }
}

void execute_binary(char binary_file[MAX_LINE]) {

    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    pid_t pid;

    binary_file[strcspn(binary_file, "\n")] = 0;  /* Strip newline character */

    /* Tokenize the input string into args array */
    char *token = strtok(binary_file, " ");
    int i = 0;
    while (token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }

    args[i] = NULL;  /* Null terminate the array */

    /* Fork a child process to execute the command */
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed.\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        /* Child process */
        if (execvp(args[0], args) < 0) {
            fprintf(stderr, "Execution failed.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        /* Parent process */
        wait(NULL);
    }
}

int delete_file(const char *path) {
    if (remove(path) == -1) {
        perror("Error removing file");
        return -1;
    }
    return 0;
}

void delete_directory(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char new_path[MAX_LINE];
        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (lstat(new_path, &st) == -1) {
            perror("Error getting file status");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            delete_directory(new_path);
        } else {
            if (remove(new_path) == -1) {
                perror("Error removing file");
            }
        }
    }

    if (rmdir(path) == -1) {
        perror("Error removing directory");
    }

    closedir(dir);
}

void my_ls(char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\t", entry->d_name);
    }
    printf("\n");

    closedir(dir);
}

char *splice_string(const char *str, int length) {
    size_t len = strlen(str);
    if (len < (size_t)length) {
        return strdup(str);
    }
    char *spliced = malloc(len - length + 1);
    if (spliced == NULL) {
        perror("Error allocating memory");
        return NULL;
    }
    strncpy(spliced, str + length, len - length);
    spliced[len - length] = '\0';
    return spliced;
}

int create_directory(const char *path) {
    int result = mkdir(path, 0777);
    if (result != 0) {
        perror("mkdir");
        return -1;
    }
    return 0;
}

char *get_pwd() {
    static char pwd[MAX_LINE];

    if (getcwd(pwd, sizeof(pwd)) == NULL) {
        perror("getcwd() error");
        return NULL;
    }

    return pwd;
}

int touch_file(const char *name) {
    // Specify the file name and mode ("a" for append)
    FILE *file = fopen(name, "a");

    if (file == NULL) {
        perror("Error creating file");
        return 1; // Return an error code
    }

    // Close the file without writing anything
    fclose(file);

    printf("File \"%s\" created successfully.\n", name);
}

void cat_file(const char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int c;
    while ((c = fgetc(file)) != EOF) {
        putchar(c);
    }

    fclose(file);
}

int main() {

    int run_flag = 1;
    char response[MAX_LINE];
    Stack history;
    initialize(&history);

    while(run_flag) {
        printf(BBLK "%s@%s$ " BBLK, getenv("USER"), get_pwd());

        char input[MAX_LINE];
        gets(input, MAX_LINE);

        input[strcspn(input, "\n")] = '\0';

        if (input[0] == '.' && input[1] == '/') {
            push(&history, input);
            execute_binary(input);
        } 
        
        else if (strcmp(input, "pwd") == 0) {
            push(&history, input);
            printf("%s\n", get_pwd());

        } 

        else if(strstr(input, "echo") != NULL) {
            char text[MAX_LINE];
            strcpy(text, splice_string(input, 5));
            printf("%s\n", text);

        }
        
        else if (strcmp(input, "history") == 0) {
            int i;
            for (i = 0; i <= history.top; i++) {
                printf("%d: %s\n", i + 1, history.data[i]);
            }
        } 
        
        else if (strcmp(input, "ls") == 0) {
            push(&history, input);
            my_ls(".");
        } 
        
        else if (strstr(input, "mkdir") != NULL) {
            push(&history, input);
            char folder_name[MAX_LINE];
            strcpy(folder_name, splice_string(input, 6));
            create_directory(folder_name);
        } 
        
        else if (strstr(input, "cd") != NULL) {
            push(&history, input);
            char dir_name[MAX_LINE];
            strcpy(dir_name, splice_string(input, 3));
            cd(dir_name);
        } 

        else if (strstr(input, "rm") != NULL) {
            push(&history, input);

            if (strstr(input, "-r") != NULL) 
            {
                char dir_to_remove[MAX_LINE];
                strcpy(dir_to_remove, splice_string(input, 6));
                delete_directory(dir_to_remove);
            } 
            
            else 
            {
                char file_to_remove[MAX_LINE];
                strcpy(file_to_remove, splice_string(input, 3));
                delete_file(file_to_remove);
            }
        }

        else if (strstr(input, "file") != NULL) {
            push(&history, input);
            char file_name[MAX_LINE];

            if (strstr(input, "size") != NULL) 
            {
                strcpy(file_name, splice_string(input, 10));
                file_info_t file_info_val = file_info(file_name);
                printf("> %ldB\n", file_info_val.file_size);
            } 
            
            else if (strstr(input, "owner") != NULL) {
                strcpy(file_name, splice_string(input, 11));
                file_info_t file_info_val = file_info(file_name);
                printf("> %s\n", file_info_val.owner);
            } 
            
            else if (strstr(input, "group") != NULL) {
                strcpy(file_name, splice_string(input, 11));
                file_info_t file_info_val = file_info(file_name);
                printf("> %s\n", file_info_val.group);
            }  
        
            else 
                {
                    strcpy(file_name, splice_string(input, 5));
                    file_info_t file_info_val = file_info(file_name);
                    printf("> %s\n", file_info_val.file_type);
                }
        }

        else if(strstr(input, "touch") != NULL)
        {
            push(&history, input);
            char file_name[MAX_LINE];
            strcpy(file_name, splice_string(input, 6));
            touch_file(file_name);
        }

        else if(strstr(input, "cat") != NULL)
        {
            push(&history, input);
            char file_name[MAX_LINE];
            strcpy(file_name, splice_string(input, 4));
            cat_file(file_name);
        }

        else if(strcmp(input, "exit")) {

            exit(0);
        } 
        
        else
        {
            push(&history, input);
            printf(RED "INVALID COMMAND" RED "\n");
        }
    }

    return 0;
}
