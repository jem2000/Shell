#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

#define TRUE 1

int global_and = 0;

void execute(char *parameters[]);
int find_char(char input[], char * parameters[]);

void remove_spaces(char* str) { //logic from https://codeforwin.org/2016/04/c-program-to-trim-both-leading-and-trailing-white-spaces-in-string.html

        int leading_spaces = 0, cur = 0;
        while(str[leading_spaces] == ' ')
        {
            leading_spaces++;
        }
        while(str[cur + leading_spaces] != '\0')
        {
            str[cur] = str[cur + leading_spaces];
            cur++;
        }

        str[cur] = '\0';
        cur = 0;
        int trailing_spaces = -1;
        while(str[cur] != '\0')
        {
            if(str[cur] != ' ')
                trailing_spaces = cur;
            cur++;
        }
        str[trailing_spaces + 1] = '\0';
    }
int parser(char* input, char *parameters[], char *delim) {
    input = strtok(input, "\n");
    char *ptr = strtok(input, delim);
    int count = 0;
    while (ptr != NULL) {
        parameters[count] = ptr;
        remove_spaces(parameters[count]);
        //printf("'%s'\n", ptr);
        ptr = strtok(NULL, delim);
        count++;
    }
    parameters[count] = NULL;

    return count - 1;
}

void less_than(char input[], char *parameters[]) {
    int count = parser(input, parameters, "<");
    //char * paramCopy[32];
    //for (int j = 0; j < count; j++) {
    //    paramCopy[j] = parameters[j];
    //}
    //paramCopy[count + 2] = NULL;
    char *file_name = parameters[count];
    parameters[count] = NULL;
    //count = parser(paramCopy[0], paramCopy, " ");
    //paramCopy[count + 1] = "NULL";
    //open file, first system call
    int file_desc = open(file_name, O_RDONLY);
    if (file_desc < 0)
        printf("Error, couldn't open file");
    int stdin_temp = 4;     //copy stdin
    dup2(STDIN_FILENO, stdin_temp);
    close(STDIN_FILENO);     //close stdin
    dup2(file_desc, STDIN_FILENO);     //dup the file descriptor for file name onto stdin
    //char temp[128];
    //fgets(temp, 128, stdin);
    //paramCopy[count + 1] = temp;
    //parser(temp, paramCopy + count + 1, " ");
    execute(parameters);
    close(file_desc);     //close the file descriptor for file name

    dup2(stdin_temp, STDIN_FILENO); //restore stdin
}
void greater_than(char input[], char *parameters[]) {
    int count = parser(input, parameters, ">");
    char * paramCopy[32];
    for (int j = 0; j <= count; j++) {
        paramCopy[j] = parameters[j];
    }
    parser(paramCopy[0], paramCopy, " ");
    find_char(input, paramCopy);
    char *file_name = parameters[count];
    int file_desc = open(file_name, O_CREAT | O_RDWR, S_IRUSR);
    parameters[count] = NULL;
    if (file_desc < 0)
        printf("Error, couldn't open file");
    int stdout_temp = 4;     //copy stdout
    dup2(STDOUT_FILENO, stdout_temp);
    close(STDOUT_FILENO);
    dup(file_desc);
    execute(paramCopy);
    close(file_desc);
    dup2(stdout_temp, STDOUT_FILENO); //restore stdout
}
void pipe_command(char input[], char *parameters[]) {

    int count = parser(input, parameters, "|");
    char * paramCopy[32];
    char * paramCopy2[32];

    int pipefd[2];
    int res;
    pid_t pid, pid2;
    res = pipe(pipefd);
    if(res != 0){
        perror("Error in pipe(): ");
    }

for (int i = 0; i < count; i++) {
    for (int j = 0; j <= count; j++) {
        paramCopy[j] = parameters[j];
    }
    for (int k = 0; k <= count; k++) {
        paramCopy2[k] = parameters[k + 1];
    }
    //char *finder_ptr = strrchr(paramCopy[i], ' ');
    //if (finder_ptr) {
        parser(paramCopy[i], paramCopy, " ");
        parser(paramCopy2[i], paramCopy2, " ");
    find_char(input, paramCopy);
    find_char(input, paramCopy2);
    //}
    //finder_ptr = NULL;
    signal(SIGCHLD, SIG_IGN);
    pid = fork();
    if (pid == 0) {
        //CHILD 1
        close(pipefd[0]);
        int stdout_temp = 5;
        dup2(STDOUT_FILENO, stdout_temp);
        close(STDOUT_FILENO);
        dup(pipefd[1]);
               //dup2(stdout_temp, STDOUT_FILENO); //restore stdout
        printf("Child  1");
        execvp(paramCopy[0], paramCopy);
        exit(0);
    }
    else if (pid > 0) {
        //PARENT
        signal(SIGCHLD, SIG_IGN);
        pid2 = fork();
        if (pid2 == 0) {
            //CHILD 2
            close(pipefd[1]);
            int stdin_temp = 5;     //copy stdin
            dup2(STDIN_FILENO, stdin_temp);
            close(STDIN_FILENO);     //close stdin
            dup(pipefd[0]);     //dup the file descriptor for file name onto stdin

            //make sure the buffer is empty and no stale content
            //print the message we just got
            printf("Child  2 ");
            execvp(paramCopy2[0], paramCopy2);
            dup2(stdin_temp, STDIN_FILENO); //restore stdin
            exit(0);
        }
        close(pipefd[1]);
        close(pipefd[0]);     //close the file descriptor for file name
        if (global_and == 0)
            wait(NULL);
    }
    else {
        perror("Error in fork(): ");
    }
}
}

void type_prompt() {
printf("my_shell$ ");
}


int find_char(char input[], char *parameters[]) {
    char *finder_ptr = strrchr(input, '&');
    if (finder_ptr) {
        parser(input, parameters, "&");
        global_and = 1;
        //is_special_char = 1;
    }
    finder_ptr = NULL;

    finder_ptr = strrchr(input, '<');
    if (finder_ptr) {
        less_than(input, parameters);
        //is_special_char = 1;
        return 1;
    }
    finder_ptr = NULL;
    finder_ptr = strrchr(input, '>');
    if (finder_ptr) {
        greater_than(input, parameters);
        //is_special_char = 1;
        finder_ptr = NULL;
        return 1;
    }
    finder_ptr = NULL;
    finder_ptr = strrchr(input, '|');
    if (finder_ptr) {
        pipe_command(input, parameters);
        //is_special_char = 1;
        return 1;
    }
    finder_ptr = NULL;

    return 0;
}

int read_command(char command[], char *parameters[]) {
    int is_special_char = 0;
    char *input = fgets(command, 32, stdin);
    if (input == 0)
        exit(0);

    is_special_char = find_char(input, parameters);

    if (!is_special_char) {
        parser(input, parameters, " ");
    }
    if (is_special_char == 1)
        return 1;
    return 0;
}
void execute(char *parameters[]) {
    signal(SIGCHLD, SIG_IGN);
    if (fork() != 0) {
        //parent code
        if (global_and == 0)
            waitpid(-1, NULL, 0);
    } else {
        //child code
        execvp(parameters[0], parameters);
    }
}

int main(int argc, char* argv[]) {

    char command[64] = "\0";
    char *parameters[32];
    int prompt = 1;
    while (TRUE) {

        if (argc == 2) {
            if (strcmp(argv[1], "-n") == 0)
                prompt = 0;
        }
        if (prompt)
            type_prompt();

        int executed = read_command(command, parameters);
        if (executed == 0)
            execute(parameters);
        global_and = 0;
    }
}