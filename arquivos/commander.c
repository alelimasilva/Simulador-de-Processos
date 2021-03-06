#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
// arquivo semelhante ao do portal didático de fork e dup2
int main(){
    int writepipe[2] = {-1, -1};
    pid_t childpid;

    if(pipe(writepipe) < 0){
        perror("pipe");
        exit(1);
    }

    if((childpid = fork()) == -1){
        perror("fork");
    }

    if(childpid == 0){
        close(writepipe[1]);

        dup2(writepipe[0], STDIN_FILENO);
        close(writepipe[0]);
        execlp("./manager", "", NULL);

    } else{
		close(writepipe[0]);

		char comando;
		do{
			scanf("%c", &comando);
			write(writepipe[1], &comando, 1);
			sleep(1);
		}while(comando != 'T');
	}

    wait(0);
}
