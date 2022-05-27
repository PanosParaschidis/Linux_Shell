/* @author Panos Paraschidis, 3164
	csd3164@.csd.uoc.gr
*/

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include  <sys/types.h>
#include <sys/wait.h> 

#define BUFFER_SIZE 512
#define TOK_BUFFER_SIZE 64
#define DELIM " \r\t\n\"\=\a"  
char buf[PATH_MAX];
char logbuf[50];
int saved_stdout, saved_stdin;

typedef struct node{
	char *var;
	char *value;
	struct node *next;
}node_t;

node_t *head;

void type_prompt(void){
	getcwd(buf, sizeof(buf));
	printf("%s@cs345sh/%s/$",getlogin(), buf);
} 

int shell_cd(char **args){
	if (args[1]==NULL){
		fprintf(stderr,"no argument for cd\n");
		}
	else{
		int x=chdir(args[1]);
		if(x!=0){
			fprintf(stderr,"can't open %s directory\n", args[1]);
		}
	}
	return 1;
}


int shell_exit(){
	exit(EXIT_SUCCESS);
}

char *read_command(void){
	int pos=0;
	int ch;
	int buf_size=BUFFER_SIZE;
	char* buffer=malloc(sizeof(char)*buf_size);
	
	if(!buffer){
		printf("memory not allocated for buffer\n");
		exit(EXIT_FAILURE);
	}
	
	while(1){
		ch=getchar();
		if(ch=='\n' || ch==EOF){
			buffer[pos]='\0';
			return buffer;
		}else{
			buffer[pos]=ch;
		}
		pos++;
	
	if(pos>=buf_size){
		buf_size=buf_size+BUFFER_SIZE;
		buffer=realloc(buffer,buf_size);
		if(!buffer){
			printf("memory not reallocated for buffer\n");
			exit(EXIT_FAILURE);
			}
		}
	}
}



char **command_token(char *command){
	int buf_size=TOK_BUFFER_SIZE;
	int pos=0;
	char **tokens=malloc(buf_size*sizeof(char*));
	char *token;
	
	
	if(!tokens){
		printf("memory not allocated for tokens\n");
		exit(EXIT_FAILURE);
	}
	
	token=strtok(command,DELIM);
	while(token!=NULL){
		tokens[pos]=token;
		pos++;
		token=strtok(NULL,DELIM);
	

	}
	tokens[pos]='\0';
	return tokens;
}

int searchLvt(char *var){
	node_t *iter;
	iter=head;
	
	if(head==NULL){
		
		return 0;
	}
	while(iter!=NULL){
		
		if(strcmp(iter->var,var)==0){
			
			return 1;
		}
		iter=iter->next;
	}
	return 0;
}

int addLvt(char *var, char *value){
	node_t *curr=(node_t*)malloc(sizeof(node_t));
	node_t *iter;
	curr->var=strdup(var);
	curr->value=strdup(value);
	
	if(head==NULL){
		
		curr->next=NULL;
		head=curr;
		printlvars();
		return 1;
	}else{
		
		iter=head;
		while(iter->next!=NULL){
			iter=iter->next;
		}
		iter->next=curr;
		curr->next=NULL;
	}
	
	return 1;
}

int removeVar(char *var){
	node_t *iter=head;
	node_t *prev=iter;
	if(strcmp(iter->var,var)==0){
		head=iter->next;
		free(iter);
		return 1;
	}
	iter=iter->next;
	while(iter!=NULL){
		if(strcmp(iter->var,var)==0){
			prev->next=iter->next;
			free(iter);
			return 1;
		}
		iter=iter->next;
		prev=prev->next;
	}
	return 1;
}

int printlvars(){
	node_t *iter;
	iter=head;
	if(iter==NULL){
		printf("Local Variable Table is empty\n");
		return 1;
	}
	while(iter!=NULL){
		printf("variable: %s	value: %s\n", iter->var,iter->value);
		iter=iter->next;
	}
	return 1;
}


int beginf(char **command){
	
	int buf_size=TOK_BUFFER_SIZE;
	char **pipe_command=malloc(buf_size*sizeof(char*));
	int pipefd[2];
	int in=0;
	int out=0;
	int i=0;
	int flag=0;
	int y=0;
	node_t *iter=head;
	int fdi, fdout;
	int pflag=0;
	
	if(command[0]==NULL){
		printf("\n");
		return 1;
	}
	if(strcmp(command[0],"exit")==0){
			shell_exit();
	}
	
		while(command[i]!=NULL){
		if(strstr(command[i], "|")){
			pflag=1;
			
		}
		i++;
	}
	i=0;
		
		while(command[i]!=NULL){
			if(strcmp(command[i],"<")==0){
			in=1;
		}
		if(strcmp(command[i],">")==0){
			out=1;
		}
		i++;
	}
	int backgr=0;
	--i;
	if(strcmp(command[i],"&")==0){
		backgr=1;
		command[i]=" ";
	}

	
	

	if(strcmp(command[0],"cd")==0){
			return (shell_cd(command));
		}


	pid_t pid, wpid;
	int status;
	
	pid=fork();
if (pid==0){
	if(pflag){
		int j=0;
		i=0;
		while(strcmp(command[i],"|")!=0) i++;
		command[i++]='\0';
		while(command[i]!=NULL){
			pipe_command[j]=command[i];
			j++;
			i++;
		}
		pipe_command[j]='\0';
		pipe(pipefd);
		if(fork()==0){
			close(pipefd[0]);
			dup2(pipefd[1],1);
			close(pipefd[1]);
			execvp(command[0], command);
		}
		else{
			close(pipefd[1]);
			dup2(pipefd[0],0);
			close(pipefd[0]);
			execvp(pipe_command[0], pipe_command);
		}
	}

	
		
	if(strcmp(command[0],"set")==0){
			if(searchLvt(command[1])==1){
				printf("var already set\n");
				return 1;
	
			}
			else{
				
				int x=addLvt(command[1],command[2]);
				return	x;
			}
	} 
	else if(strcmp(command[0],"unset")==0){
			if(searchLvt(command[1])==0){
				printf("var wasn't set\n");
				return 1;
			}
			else{
				return (removeVar(command[1]));
			}
	}
	else if(strcmp(command[0],"printlvars")==0){
			if(out!=0){
			
			i=0;
			while(strcmp(command[i],">")!=0) i++;
			command[i++]='\0';
			if((fdout=open(command[i], O_CREAT | O_TRUNC | O_WRONLY, 0600))<0){
				printf("Couldn't open output file");
				exit(EXIT_FAILURE);
			}
			command[i]='\0';
			dup2(fdout,1);
			close(fdout);
			out=0;
			printlvars();
			return 1;
			}
			else{
					return (printlvars());
				}
	}
	else{
		if(in!=0 && out!=0){
			
		
			i=0;
			while(strcmp(command[i],"<")!=0) i++;
			command[i++]='\0';
			if((fdi=open(command[i],O_RDONLY))<0){
				printf("Couldn't open input file");
				exit(EXIT_FAILURE);
			}
			command[i]='\0';
			dup2(fdi,0);
			close(fdi);
			in=0;

			i=0;
			while(strcmp(command[i],">")!=0) i++;
			command[i++]='\0';
			if((fdout=open(command[i], O_CREAT | O_TRUNC | O_WRONLY, 0600))<0){
				printf("Couldn't open output file");
				exit(EXIT_FAILURE);
			}
			command[i]='\0';
			dup2(fdout,1);
			close(fdout);
			out=0;

			execvp(command[0],command);
			
			}
		else if(in!=0){
			i=0;
			while(strcmp(command[i],"<")!=0) i++;
			command[i++]='\0';
			if((fdi=open(command[i],O_RDONLY))<0){
				printf("Couldn't open input file");
				exit(EXIT_FAILURE);
			}
			command[i]='\0';
			dup2(fdi,0);
			close(fdi);
			in=0;
			execvp(command[0],command);
			}
		else if(out!=0){
			i=0;
			while(strcmp(command[i],">")!=0) i++;
			command[i++]='\0';
			if((fdout=open(command[i], O_CREAT | O_TRUNC | O_WRONLY, 0600))<0){
				printf("Couldn't open output file");
				exit(EXIT_FAILURE);
			}
			command[i]='\0';
			dup2(fdout,1);
			close(fdout);
			out=0;
			execvp(command[0],command);
			
			}
		while(iter!=NULL){
			if(strcmp(iter->var,command[0])==0){
				command[0]=strdup(iter->value);
			}
			iter=iter->next;
		}
			execvp(command[0],command);
			
		
	}
}else{

		
		wait(&status);
		
	
}
	return 1;

}





void loopa(){
	char *command_line;
	char **command;
	int status;
	
	do{
		type_prompt();
		command_line=read_command();
		command=command_token(command_line);
		
		

		status=beginf(command);
		free(command);
		free(command_line);
	}while(status);
	
}

int main(){
	
	loopa();
	
	return EXIT_SUCCESS;
}
