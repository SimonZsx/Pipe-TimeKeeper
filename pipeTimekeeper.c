
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

int s=-1;
char fn1[3000],fn2[3000];
void mask_during_handle(sigset_t * set){
	sigaddset(set, SIGQUIT);
};
void sigusr1_handler(int signum){ //, siginfo_t *sig, void *v
	//printf("SIGUSR1 received,start execute command\n");
	s=1;
};	
void sigint_handler(int signum){
	exit(0);	
};

void sigint_handler1(int signum){
	//exit(0);
};

const char * getsigname(int signum){

const char * names[50]={"NULL","SIGHUP","SIGINT","SIGQUIT","SIGILL","SIGTRAP","SIGABRT","SIGBUS","SIGFPE","SIGKILL","SIGUSR1","SIGSEGV","SIGUSR2","SIGPIPE","SIGALRM","SIGTERM","SIGSTKFLT","SIGCHLD","SIGCONT","SIGSTOP","SIGTSTP"};
return(names[signum]);
}

/* Split a string to a string array, with string segment sepearted by space stored in an array of strings
	parameters:
		input:	the string to split
	retrun value:
		the string array with string segment seperated by space
*/
char** stringToArray(char* input){
	char* str = input;	
	char ** output = (char**) malloc(sizeof(char*) * (strlen(input)+1));
	char * pch = strtok(str, " \t\n");
	int count = 0;
	while (pch != NULL){
		output[count++] =  pch;
		pch = strtok(NULL, " \t\n");
	}
	output[count] = NULL;
	return output;
}


int main(int argc, char** argv){


	char* myargs[100][100];

	int i=1;
	int k=0;
	int m=0;
	int fd_in;

	while(i<argc){
		if(strcmp(argv[i],"!")==0){k++;m=0;i++;}
		else{
		myargs[k][m]=argv[i];
		i++;m++;}		
	}


	// TODO: Create the child process. 
	signal(SIGUSR1,sigusr1_handler);
        pid_t pid;
	int h=0;

	int mypipe[2];


        while(h<=k)

        {
	pipe(mypipe);
        pid=fork();
	// TODO: Branch the program routine for parent and child
        if(pid>0) //parent
        {
		signal(SIGINT, sigint_handler1);

		int status;

		struct timespec start;
		struct timespec end;

		close(mypipe[1]);
        	fd_in = mypipe[0];
		clock_gettime(CLOCK_REALTIME,&start);
			
		siginfo_t* infop;

		

		if(kill(pid,SIGUSR1)==0){
			//printf("Process with id: %d created for the command: %s\n",pid,myargs[h][0]);	
		}

		waitid(P_PID,pid, infop, WEXITED|WNOWAIT);

		clock_gettime(CLOCK_REALTIME,&end);

                FILE * stat;
		FILE * fstatus;

		sprintf(fn1,"/proc/%d/stat",pid);
		stat=fopen(fn1,"r");
		if(stat==NULL){
			printf("Error when open stat file\n");
			exit(-1);
		}

		char str[300];

		fgets(str,sizeof(char)*300,stat);

		char** statget = stringToArray(str);
		double count = sysconf(_SC_CLK_TCK);
		fclose(stat);
		sprintf(fn2,"/proc/%d/status",pid);
        	fstatus = fopen(fn2, "r");
		if(fstatus==NULL){
			printf("Error when open status file\n");
			exit(-1);
		}
		char* strs=NULL;
		size_t len=0;
		int totalsw = 0;	
		int added = 0;		
		while(getline(&strs, &len, fstatus)!=-1){
			if(!strstr(strs, "ctx"))
				{continue;printf("sss111\n");}
			else {
				char* p;
				p = strstr(strs, "ctxt");
				char** temp = stringToArray(p); 
				//printf("switch1:%d \n",atol(temp[1]));
				totalsw += atol(temp[1]);}
		}	
		fclose(fstatus);		


		waitpid(pid,&status,0);
		if(WIFSIGNALED(status)==1){
		printf("The command %s is interrupted by the signal number = %d (%s) \n",myargs[h][0],WTERMSIG(status),getsigname(WTERMSIG(status)));
		}
		else{
		printf("The child process %s has returned with code = %u\n",myargs[h][0],WEXITSTATUS(status));
		}
		double time = (double)(end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1000000000.0;
		printf ("Real: %.3fs ", time);
		printf("user: %.3fs ", atol(statget[13])/count);
		printf("system: %.3fs ", atol(statget[14])/count);
		printf("context switch: %d\n", totalsw);


	}
	else if(pid==0) //child
	{
          	printf("Process with id: %d created for the command: %s, h: %d\n", getpid(), myargs[h][0], h);
          	dup2(fd_in, 0); 
          	if (k>0) //if there is a next cmd
          		{
			if(h<k){dup2(mypipe[1], 1);}
			//if(h!=k){			
			//close(1);}
          		close(mypipe[0]);}
          	if(execvp(myargs[h][0], myargs[h])==-1){
          		perror("Error: ");
          		exit(1);
          		}
	}
	else{
	
	} 

	h++;
	} 
	return 0;
}



