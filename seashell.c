#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "linked_list.h"
#include "tokenizer.h"
#include "jobs_recycle_list.h"
#define TRUE 1
#define BSIZE 1024
#define INPUT 0
#define OUTPUT 1
#define MAX_ARGS 100
#define MAX_ARG_LEN 1040
int pid; //process id
int status; //status of process
int num_bytes;
int m; //to keep track of index of tokens
int bg =0; //boolean to see if bg was typed as command
int fg = 0; //boolean to see if fg was typed as command
int is_pipe = 0; //boolean to see if pipe command was called
int background = 0; //boolean to see if process was ran in background
int is_break = 0;  //boolean to see if first token is null --> if so then break out of the loop
char *tokens[MAX_ARGS]; //array to keep track of all tokens from commands typed into terminal

struct Node* bg_list; //background process linked list
struct Node* fg_list; //foreground process linked list
struct Node2* jobs_id_recycle; //linked list to keep track of job identifiers
struct Node* popped; //node to keep track of popped nodes from process lists
char *cmd; //pointer to keep track command typed in terminal
int count = 1; //if jobs_id_recycle is null, use this to identify jobs
int background_b = 0; //boolean to see if process is ran in background becasue of command bg
int pipe_done = 0; //boolean to see if pipe is done executing
int job; // identify job process 

int remove_foreground; //boolean to remove specific job when bg and job identifier are commands
int remove_background; //boolean to remove specific job when fg and job identifier are commands
void handler(int); //handle signals


/*Signal handler*/
void handler(int signum){
	//catch ctrl z
    if(signum == SIGTSTP){
  		write(STDOUT_FILENO, "\n Stopped: ", 9);
  	}

  	//catch ctrl c
  	if (signum == SIGINT) {
  		popped = pop(&fg_list);
  		jobs_id_recycle = push_job(&jobs_id_recycle, popped->job_id);

  	}

  	//catch sigchld--> if child was executed
  	if(signum == SIGCHLD){
		waitpid(pid, &status,  WNOHANG | WSTOPPED);

		//if child process exits normally
		if(WIFEXITED(status)){
			//if background bg was presed --> need to run in background 
			if (background_b) {
				// pop process because it is done
				popped = pop(&bg_list); 
				//push popped id to jobs_id_recycle list to recycle for later
				jobs_id_recycle = push_job(&jobs_id_recycle, popped->job_id);
				printf("\nFinished: %s\n", popped->name);
				write(STDOUT_FILENO, "penn-sh> ", 9);
				background_b = 0;
			 }
			 //if piping process
			else if (is_pipe){
				//is the pipe done?
				if (pipe_done) {
					if (fg_list != NULL) {
						//pop process because it is done
						popped = pop(&fg_list);
						//push popped id to jobs_id_recycle list to recycle for later
						jobs_id_recycle = push_job(&jobs_id_recycle, popped->job_id);
					}
					//reset variables
					background_b = 0;
					is_pipe = 0;
					pipe_done = 0;
				}
			}
			//if foreground process
			else {		
				//reset variables		
				background_b = 0;
			}
		}

		//waiting for process to stop if bg and fg were never pressed (ie ctrl z)
		if (WIFSTOPPED(status) && bg==0 && fg==0) {
			//if process is background process
			if (background == 1) {
				//update status
				bg_list->status = "stopped";
				write(STDOUT_FILENO, "Stopped: \n", 10);
			}
			else {
				if (fg_list != NULL) {
					//pop from foreground
					popped = pop(&fg_list);
					//push into background
					bg_list = push(&bg_list, popped->pid, &popped->name, "stopped", popped->job_id);
				}
			}
	 	}
	}
}


/*to parse the tokens from the shell*/
void get_tokens(TOKENIZER *tokenizer, int file_d[2]) {
	//keep track of each token
	char* token;
	//loop until you see null token
	while((token = get_next_token(tokenizer)) != NULL){		
		//look for redirecion tokens
		if((token[0] == '>') | (token[0] == '<')){
			//to keep track of tokens after redirection
			char* next_token; 
			if((next_token = get_next_token(tokenizer)) != NULL){
				if(token[0] == '>' ){
					//set token as standard out
					int new_stout = open(next_token,O_CREAT|O_WRONLY|O_TRUNC , 0644);
					if (new_stout < 0) {
						continue;
					}
					//duplicate stdout and closes it
					dup2(new_stout,STDOUT_FILENO);
				}
				if(token[0] == '<'){
					//token is standard in file
					int new_stin = open(next_token,O_RDONLY);
					if (new_stin < 0) {
						continue;
					}
					//duplicate stdin and closes it
					dup2(new_stin,STDIN_FILENO);
				}
			}
			else{
				continue;
			}
		}
		//look for piping  command
		else if (token[0] == '|') {
			pipe_done = 0;
			is_pipe = 1;
			int pid_2;
			//creates a pipe from file descriptor
			pipe(file_d);
			pid_2 = fork();
			//child process
			if (pid_2 == 0) {
				//dup std out 
				dup2(file_d[1], 1);
				//close read because we don't need this yet
				close(file_d[0]);
				//execute child process
				execvp(tokens[0], tokens);
			}
			//parent process
			else if (pid_2 > 0) {
				//make all the tokens null so we can keep track of everything after pipe
				for(int u = 0; u < MAX_ARGS; u++){
					tokens[u] = NULL;  
					m = 0;
				}
				//close write
				close(file_d[1]);
				//wait for child process
				waitpid(pid_2, &status, WNOHANG|WSTOPPED);
			}
			else {
				continue;
			}

		}
		//if command is fg
		else if(token[0] == 'f' && token[1] == 'g'){
			fg=1;
			char *next_token;
			while ((next_token = get_next_token(tokenizer)) != NULL) {
				//look for job identifier that needs to bring to foreground
				job = next_token[0] - '0';
				if (bg_list != NULL) {
					printf("restarting: %s\n", get_name(bg_list, job));
				}
				remove_foreground = 1;
			}
		}
		//if command is bg
		else if(token[0] == 'b' && token[1] == 'g'){
			bg=1;
			char *next_token;
			while ((next_token = get_next_token(tokenizer)) != NULL) {
				//look for job identifier that needs to bring to background
				job = next_token[0] - '0';
				if (bg_list != NULL) {
					printf("restarting: %s\n", get_name(bg_list, job));
				}
				remove_background = 1;
			}
		}
		// if command is jobs
		else if(token[0] == 'j' && token[1] == 'o' && token[2] == 'b' &&  token[3] == 's'){
			//loop through background process list and print job id, name, and status (running stopped)
			struct Node *curr = bg_list;
			while(curr != NULL) {
				printf("[%d] %s (%s)\n", curr->job_id, curr->name, curr-> status);
				curr = curr->next;
			}
			//loop through foreground process list and print job id, name, and status (running stopped)
			curr = fg_list;
			while(curr != NULL) {
				printf("[%d] %s (%s)\n", curr->job_id, curr->name, curr-> status);
				curr = curr->next;
			}
		}
		//store the rest of the tokens to be executed for later
		else{
			tokens[m] = token;
			m++;
		}
	}
	//if the command bg was typed
	if(bg){
		struct Node *temp = NULL;
		if (bg_list == NULL) {
			return;
		}
		//if need to remove specific job
		if (remove_background) {
			//remove specific node that has this job identfier
			temp = remove_node(&bg_list, job);
			//push it back on bg list but now it's status is running
			bg_list = push(&bg_list, temp->pid, &temp->name, "running", temp->job_id);
			//continue where it left off
			kill(bg_list->pid, SIGCONT);
			bg = 0;
			remove_background = 0;
			//background bg is executing
			background_b = 1;
		}
		else {
			//if only bg was typed
			printf("Running: %s \n", bg_list->name);
			//change status
			bg_list->status = "running";
			//continue where it left off
			kill(bg_list->pid,SIGCONT);
			bg = 0; 
			background_b = 1;
		}				
	}
	//if the command fg was typed
	if(fg){
		if (bg_list == NULL) {
			return;
		}
		else {			
			background_b = 0;
			struct Node *temp = NULL;
			//if a specific job id was typed
			if (remove_foreground) {
				temp = remove_node(&bg_list, job);
			}
			//if only fg was typed
			else {
				temp = pop(&bg_list);
			}
			//push onto fg list and change status
			fg_list = push(&fg_list, temp->pid, &temp->name, "running", temp->job_id);
			//continue where left off
	    	kill(fg_list->pid, SIGCONT);
	    	//wait for job to complete
	    	waitpid(fg_list->pid, &status, WUNTRACED);
	    	//once job completes pop off fg list
	    	pop(&fg_list);
			fg= 0;
			remove_foreground = 0;
		}
	}
}

/*deals with parent and child processes*/
void parent_child(int file_d[2], int this_fd) {
	pid = fork();
	//child process
	if(pid == 0){
		signal(SIGTERM, SIG_DFL);		  
        signal(SIGINT, handler);
        signal(SIGTSTP, handler);
        //if pipe was command
		if(is_pipe ==1){
			//dup other end of file descritpor
			dup2(file_d[0], 0);  
		}  
		//execute output of the other end of pipe as input of this end
		execvp(tokens[0], tokens);

	}
	//error in fork
	else if(pid <  0){
		perror("error in fork");
	}
	//parent process
	else {
		//piping is done!		
		pipe_done = 1;

		//set child group id to itself
		if (setpgid(pid, pid) < 0){
			perror ("setpgid child error");
		}

		//if foreground process
		if (background == 0) {
			int j;
			//give terminal control to child
			tcsetpgrp(this_fd, pid);
			//if there are recycled job identifiers waiting to be used
			if (jobs_id_recycle != NULL) {
				j = pop_job(&jobs_id_recycle);
				//push process onto fg list
				fg_list = push(&fg_list,pid, tokens, "running", j);
			}
			//if no recycled job identifiers available
			else {
				fg_list = push(&fg_list,pid, tokens, "running", count);
				count++;
			}

			//wait for child process to be done executing
			waitpid(pid, &status, WUNTRACED);

			//pop process off fg list and recycle job identifier
			if (fg_list != NULL) {
				popped = pop(&fg_list);
				jobs_id_recycle = push_job(&jobs_id_recycle, popped->job_id);
			}
			//give terminal control back to parent
			tcsetpgrp(this_fd, getpgid(getpid()));
		}
		//if background process
		else{
			signal(SIGTTIN, SIG_DFL);
			int j;
			//push process onto bg list 
			if (jobs_id_recycle != NULL) {
				j = pop_job(&jobs_id_recycle);
				bg_list = push(&bg_list,pid, tokens, "running", j);
			}
			else {
				bg_list = push(&bg_list,pid, tokens, "running", count);
				count++;
			}
			background_b = 1;
			printf("Running: %s \n",cmd);
			//wait for child to execute
			waitpid(pid, &status, WNOHANG|WSTOPPED);
		}
	}
}


/*da main event*/
int main(int argc, char* args[]) {
	TOKENIZER *tokenizer;  
	cmd = malloc(sizeof(char)* BSIZE);
	int this_stout =  dup(STDOUT_FILENO);
	int this_stin = dup(STDIN_FILENO);
	int this_fd = STDIN_FILENO;  
	int file_d[2];
	bg_list = (struct Node*)malloc(sizeof(struct Node));
	fg_list = (struct Node*)malloc(sizeof(struct Node));
	jobs_id_recycle = (struct Node2*)malloc(sizeof(struct Node2));
	bg_list = NULL;
	fg_list = NULL;
	jobs_id_recycle = NULL;

	remove_background = 0;
	remove_foreground = 0;
	
	
	//for blocking signals
	//sigset_t mask;
	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCHLD, handler);
	while(TRUE){
		m = 0;
		background = 0; 
		is_break = 0; 
		for(int u = 0; u < MAX_ARGS; u++) {
			tokens[u] = NULL; 
		} 
		write(STDOUT_FILENO, "penn-sh> ", 9);
		dup2(this_stout, STDOUT_FILENO);
		dup2(this_stin, STDIN_FILENO);

		//read command into buffer
		num_bytes =  read(INPUT, cmd, BSIZE );  
		if(num_bytes > 1024){
			printf("error: num_bytes is greater then 1024\n");
			continue;
		}
		if(num_bytes <= 1){
			continue; 
		}
		/*checks if command is to be run in the background*/
		if(cmd[num_bytes-2] == '&'){
			background = 1; 
			cmd[num_bytes-2] = '\0';
			if (cmd[0] == 'c' && cmd[1] == 'a' && cmd[2] == 't') {
				printf("Running: %s\n",cmd );
			}
		}
		else{
			cmd[num_bytes-1] = '\0';
		}

		 //parse the string
		tokenizer = init_tokenizer(cmd);

		//get tokens from command line
		get_tokens(tokenizer, file_d);
		
		//if the first token is null --> don't fork
		if(tokens[0] == NULL){
			is_break = 1;
		}
		if(is_break){
			continue;
		}
		//null terminate
		tokens[m+1] = NULL;

		//parent child 
		parent_child(file_d, this_fd);
				 
	} 
	signal(SIGTTIN, SIG_DFL);
	background = 0; 
	bg = 0; 
	close(file_d[0]);
	close(file_d[1]);
	free(cmd);
	free(bg_list);
	free(fg_list);
	free(jobs_id_recycle);
	free_tokenizer( tokenizer );
	return 1;
}