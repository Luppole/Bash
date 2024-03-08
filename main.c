#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <curses.h>

#define MAX_LINE 1024
#define MAX_SIZE  256
#define MAX_DIR 128
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

char directory_payload[MAX_DIR];

typedef struct 
{
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


int cd(const char *directory) 
{
    if (chdir(directory) != 0) 
    {
        perror("chdir");
        return 1;  // Return an error status
    }

    // Print the current working directory after changing
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Current working directory: %s\n", cwd);
        return 0;  // Return success status
    } 
    
    else 
    {
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
        return 1;
    }
        
    else if (pid == 0) {
        /* Child process */
        if (execvp(args[0], args) < 0) {
            fprintf(stderr, "Execution failed.\n");
            return 1;
        }
    }
    
    else {
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

void my_ls(char *path)
{
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

char* splice_string(const char *str, int length) {
    size_t len = strlen(str);
    if (len < 7) {
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

int main() {

    initscr();
    cbreak(); // Disable line buffering
    noecho(); // Don't display input characters
    keypad(stdscr, TRUE); // Enable special keys
    timeout(0); // Non-blocking input

    getch();
    
	int run_flag = 1;
    Stack history;
    initialize(&history);

	while(run_flag)
	{
	    printf(ANSI_COLOR_BLUE "%s@%s> " ANSI_COLOR_BLUE, getenv("USER"), get_pwd());
    	char input[MAX_LINE];
    	fgets(input, MAX_LINE, stdin);

    	input[strcspn(input, "\n")] = '\0';

    	if (input[0] == '.' && input[1] == '/')
    	{
            push(&history, input);
    	 	execute_binary(input);
    	}

    	else if (strcmp(input, "pwd") == 0) 
    	{
            push(&history, input);
        	printf("%s\n", get_pwd());
		}

        else if (strcmp(input, "history") == 0) {
            int i;
            for (i = 0; i <= history.top; i++) {
                printf("%d: %s", i+1, history.data[i]);
            }
        }

        else if(strcmp(input, "ls") == 0) 
        {
            my_ls(".");
        }

        else if(strstr(input, "mkdir") != NULL) {
            char folder_name[MAX_LINE];
            strcpy(folder_name, splice_string(input, 6));
            create_directory(folder_name); 
        }

        else if(strstr(input, "cd") != NULL) {
            char dir_name[MAX_LINE];
            strcpy(dir_name, splice_string(input, 3));
            cd(dir_name);
        }

        else if(strstr(input, "rm") != NULL) 
        {
            if(strstr(input, "-r") != NULL) 
            {
                char dir_to_remove[MAX_LINE];
                strcpy(dir_to_remove, splice_string(input, 6));
                delete_directory(dir_to_remove);
            }

            else {
                char file_to_remove[MAX_LINE];
                strcpy(file_to_remove, splice_string(input, 3));
                delete_file(file_to_remove);
            }
        }

        else {
            printf(ANSI_COLOR_RED "INVALID COMMAND" ANSI_COLOR_RED);
        }
    }
}
