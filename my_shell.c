#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define ARRAY_SIZE 2

int BACKGROUND_FLAG = 0;
int FIRST_BACKGROUND = 1;
pid_t cpid;
int bgcpid;
int ppgid;
int cstatus;


static int arrayIndex =0;

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

	if(readChar == '&'){
		BACKGROUND_FLAG = 1;
	}else if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

void insertBgTask(int pid){
	if(arrayIndex == ARRAY_SIZE){
		printf("No more background task \n");
		return;
	}else{
		arrayIndex++;
	}
}

void reapBgChild(){
	int status_bg;
		int cwpid = waitpid(-1,&status_bg,WNOHANG);
		while(cwpid > 0){
		printf("\n Shell: Background process finished \n");
		printf("[%d] Terminated \n", cwpid);
		arrayIndex--;
		cwpid = waitpid(-1,&status_bg,WNOHANG);
		}
		return;
}


void executeBin(char** tokens){
	int i;
	char *args[MAX_NUM_TOKENS +1];
	char * cmd = tokens[0];
	for(i=0;tokens[i]!=NULL;i++){
			args[i]=tokens[i];
		}

	char command[MAX_TOKEN_SIZE] = "/usr/bin/";
	strcat(command,tokens[0]);
	args[i]=NULL;
	int status;
	args[0] = command;
	

	if(strcmp(tokens[0],"cd")==0){
		if (tokens[2]){
			printf("Shell:Incorrect command \n");
		}else{
		int result = chdir(tokens[1]);	
		if(result != 0){
			printf("Shell: Directory doesnot exist \n");
		}
		
	}
	return;
	}else if(strcmp(tokens[0],"exit")==0){

		// // Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
		killpg(bgcpid,SIGKILL);
		sleep(1); //Added because kill was taking time in my system
		reapBgChild();
		kill(0,SIGKILL);
	}
	else{
		
		cpid = fork();
		if(cpid==0){
			int result =execv(args[0],args);
			if(result != 0){
				printf("Command does not exist \n");
				kill(getpid(), SIGKILL);
				waitpid(getpid(),&status,0);
			}
		}else {
			if(BACKGROUND_FLAG){

				if(FIRST_BACKGROUND ==1){
					FIRST_BACKGROUND = 0;
					bgcpid = cpid;
				}
				setpgid(cpid,bgcpid);

				if(getpgid(cpid)== ppgid){
					bgcpid = cpid;
					setpgid(cpid,bgcpid);
				}

				int pid = waitpid(cpid,&status,WNOHANG); 
				insertBgTask(pid);
			}else{
			if (setpgid(cpid,0)== -1 ){
				printf("ERROR : Unable to change process group\n");
			}
			waitpid(cpid,&status,0);
			
			}
			cpid = 0;
		}

	}

	return;
}

void siginitHandler(int d) {
	if (cpid != 0){
		killpg(cpid,SIGKILL);
		waitpid(cpid,&cstatus,0);
		cpid = 0;
		printf("/n");
	}
	return;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	char s[100];

	signal(SIGINT, siginitHandler);
	ppgid = getpgid(0);

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		BACKGROUND_FLAG = 0;
		printf("~%s", getcwd(s, 100));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

	   if(tokens[0]==NULL){
		reapBgChild();
		free(tokens[0]);
		free(tokens);
		continue;
	   }

		executeBin(tokens);

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
		reapBgChild();

	}
	return 0;
}



/*TO DO :
See what to send in kill SIGTERM/SIGKILL/SIGSTOP

*/