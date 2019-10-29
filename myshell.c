#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define MAX_CMD_ARG 20



const char *prompt = "myshell> ";
char  cmdline[BUFSIZ];
char  cmdline2[BUFSIZ];
char* cmdvector[MAX_CMD_ARG];
char* cmdvector2[MAX_CMD_ARG];
char* cmdvector3[MAX_CMD_ARG];
void fatal(char *str){
	perror(str);
	exit(1);
}

void my_cd(int cmd_num,char* cmd_vector[]){ //if cmd == cd
	char *path;
	path = (char*)getenv("HOME"); //path = environ path value
	if(cmd_num>1){
		path = cmd_vector[1]; // if has arg , set path
	}
	else if( path ==NULL){ // no env
			path = ".";
	
	}
	chdir(path);
}
void my_exit(){
	exit(0);

}

int makelist(char *s, const char *delimiters, char** list, int MAX_LIST){	
  int i = 0;
  int numtokens = 0;
  char *snew = NULL;

  if( (s==NULL) || (delimiters==NULL) ) return -1;

  snew = s + strspn(s, delimiters);	/* delimiters¸¦ skip */
  if( (list[numtokens]=strtok(snew, delimiters)) == NULL )
    return numtokens;
	
  numtokens = 1;
  
  while(1){
     if( (list[numtokens]=strtok(NULL, delimiters)) == NULL)
	break;
     if(numtokens == (MAX_LIST-1)) return -1;
     numtokens++;
  }
  return numtokens;
}
int redirectout(){
	int i;
	int fd;
	int j;

	for(i =0; cmdvector3[i]!= NULL;i++){
		if(strcmp(cmdvector3[i],">")==0){
			break;
	
		}

	}

	
	if(cmdvector3[i]!=NULL){
	if(cmdvector3[i+1]==NULL){
		fatal("no arg");
	}else{
		if((fd =open(cmdvector3[i+1],O_WRONLY|O_CREAT,0644) )==-1){
			fatal("errordeath");	
		}
	
	}
	dup2(fd,1);
	close(fd);
	cmdvector3[i] = NULL;
	cmdvector3[i+1] = NULL;
	for(i = i+2 ; cmdvector3[i]!=NULL;i++){
		cmdvector3[i-2] = cmdvector3[i];

	}	
	cmdvector3[i-2]= NULL;
	
	}
}

int redirectin(){
	int i;
	int fd;
	int j;
	for(i =0; cmdvector3[i]!= NULL;i++){
		if(strcmp(cmdvector3[i],"<")==0){
			break;
	
		}

	}
	if(cmdvector3[i]!=NULL){
		if(cmdvector3[i+1]==NULL){
			fatal("no arg");
		}
		else{
		if((fd =open(cmdvector3[i+1],O_RDONLY))==-1){
			fatal("errordeath");	
			}
		}
	
		dup2(fd,0);
		close(fd);
		cmdvector3[i] = NULL;
		cmdvector3[i+1] = NULL;
		for(j = i+2 ; cmdvector3[j]!=NULL;j++){
			cmdvector3[j-2] = cmdvector3[j];

		}	
		cmdvector3[j-2]=NULL;
		
	}
	return 0;
}

int main(){
  signal(SIGQUIT,SIG_IGN);
  signal(SIGINT,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGTTOU,SIG_IGN);
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NOCLDSTOP;
  act.sa_handler = SIG_IGN;
  sigaction(SIGCHLD,&act,NULL);
  

  char* tempv;
  int i=0;
  int cmdvector_num;
  int isbackground;
  int isredirectin=0;
  int isredirectout=0;
  int ispipe=0;
  int i2=0; 
 



  int pipecount=0;
  int status;
  pid_t pid1;
  pid_t pid2;
  while (1) {
       	int  pfd[2];   
  	fputs(prompt, stdout);
	fgets(cmdline, BUFSIZ, stdin);
	if(cmdline[0]=='\n') continue;
	
	cmdline[strlen(cmdline)-1]='\0';
	isbackground=0;
	for(i=0;i<strlen(cmdline);i++){
		if(cmdline[i] == '&')
		{
			cmdline[i]=' ';
			isbackground=1;
		}

	}
			
	isredirectin =0;
	isredirectout =0;
	
	for(i = 0 ; i< strlen(cmdline);i++){
		if(cmdline[i]=='<'){
			isredirectin=1;
			break;
		}

	}
	for(i = 0 ; i< strlen(cmdline);i++){
		if(cmdline[i]=='>'){
			isredirectout=1;
			break;
		}

	}	
	
		strcpy(cmdline2,cmdline);
		
		cmdvector_num = makelist(cmdline," \t",cmdvector, MAX_CMD_ARG );
		pipecount = makelist(cmdline2, "|", cmdvector2, MAX_CMD_ARG);

			
					
	if(strncmp(cmdline,"cd",2 )==0)		//cd
	{
		my_cd(cmdvector_num,cmdvector);
		continue;		
	}
	else if(strncmp(cmdline,"exit",4)==0)		//exit
	{
		my_exit();
	}

			switch(pid2=fork()){
			case 0:
				setpgid(0,0);
				signal(SIGQUIT,SIG_DFL);
				signal(SIGINT,SIG_DFL);
		       	  	if(isbackground==0){
					tcsetpgrp(STDIN_FILENO,getpgid(0));
				}
				for(i=0;i<pipecount-1;i++)
				{
					pipe(pfd);
					
					switch(pid1 =fork())
					{
					case 0: 
					close(pfd[0]);
					dup2(pfd[1],1);
					makelist(cmdvector2[i], " \t", cmdvector3, MAX_CMD_ARG);

					if(isredirectin){
						redirectin();
					}
					if(isredirectout){
						redirectout();
					}			      	
					execvp(cmdvector3[0], cmdvector3);
					fatal("main()");
					break;
					case -1: fatal("pipe error");break;
					default:
					close(pfd[1]);
					dup2(pfd[0],0);
					}
				}
				makelist(cmdvector2[i], " \t", cmdvector3, MAX_CMD_ARG);

				if(isredirectin){
					redirectin();
				}
				if(isredirectout){
					redirectout();				
				}
		
				execvp(cmdvector3[0], cmdvector3);
				fatal("???");	
				break;
			case -1:
  				fatal("main()");
				break;
			default:
				if(isbackground==0){
					waitpid(pid2,NULL,0);
					tcsetpgrp(STDIN_FILENO,getpgid(0));
		
				}
				else{
					usleep(1000*50);
				}
	
				fflush(stdout);	
			}
			
				
			
	
	}
  return 0;
}
