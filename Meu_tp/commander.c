#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

int main() {
    int writepipe[2] = {-1, -1};
    pid_t childpid;

    if(pipe(writepipe) < 0) {
        perror("pipe");
        exit(1);
    }

    if((childpid = fork()) == -1) {
        perror("fork");
    }

    if(childpid == 0) {
        close(writepipe[1]);

        dup2(writepipe[0], STDIN_FILENO);
        close(writepipe[0]);
        execlp("./process_commander", "./process_commander", NULL);

    } else {
		char letra = '\0';
        close(writepipe[0]);
		while(letra != 'T'){
            while(letra != 'P' || letra != 'Q' || letra != 'U' || letra != 'T'){
                scanf("%c", &letra);
                letra = toupper(letra);
            }
			write(writepipe[1], &letra, 1);
			sleep(1);
        }
    }

    wait(0);
}
