#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/socket.h>
#include <pthread.h>
#include "locker.h"

#define DEFAULT_PROTOCOL 0
#define MAXLINE 100
#define MAX_LOCKER_SIZE 50
;

int locker_size;
int **locker_info;
struct locker *locker;

char message[200];
char welcome[400];

int clnt_socks[50];
int clnt_cnt = 0;
int locker_num, locker_use, locker_conn, locker_lock, empty_locker;
char locker_pw[10];

pthread_mutex_t mutx;

int t = 60;
void alarmHandler(int signo)
{
   t -=2;

   if (t == 0)
      t = 60;
   else alarm(2);
}

void client_show_locker(int cfd)
{	
	int i;
	char message[800];
	char message1[400];

	sprintf(message, ("\n| ------------------------------현재 사물함의 정보----------------------------- |\n"
			"                            비어있음 : 0, 사용중 : 1\n"));
	for(i=1; i<locker_size+1; i++)
	{
		if((i-1)%5 == 0)
		{
			strcat(message, "\n|");
		}
		sprintf(message1, " [%d]\t%d\t|" , i, locker[i].use);
		strcat(message, message1);
	}
	strcat(message, "\n");
	send(cfd, &message, sizeof(message), 0);
	printf("%s\n", message);
	printf("전체 사물함 수 : %d, 접속자 수 : %d, 비어있는 사물함 수 : %d\n", locker_size, clnt_cnt, empty_locker);
}


void *t_function(void *data)
{
	int cfd;
	int action_num;
	
	cfd = *((int *)data);

	sprintf(welcome, ".　/＼＿／?\n"
			 "／ _ノ　ヽ_ ＼\t\t어서오세요\n"
			 "|　 　━　 ━　 i\t\tNANG 사물함입니다 :)\n"
			 "＼ミ  (_人_) 彡\n"
			 "　／￣￣⌒＼/⌒)――――――\t원하는 동작을 입력해주세요 (1, 2 or 3)\n"
			 ". /　　　＿／\t\t 1: 물건 넣기\n"
			 ". |　　　＼\t\t 2: 물건 찾기\n"
			 "　＼ 〇_　 ＼\t\t 3: 나가기\n"
			 "　　＼ノ.＼_ノ\t\t숫자를 입력하세요 : ");
			 
	
	send(cfd, &welcome, sizeof(welcome), 0);
	// 1. welcome + action_num 받기
	sprintf(message, ("welcome\n"
			 "please choose what you want\n"
			 "\t1: 사물함에 물건 넣기\n"
			 "\t2: 사물함에서 물건 찾기\n"
			 "숫자를 입력하세요: "));
	read(cfd, &action_num, sizeof(action_num));
	
	// 2. 현재 사물함의 정보 알려주기
	client_show_locker(cfd);
	
	if (action_num == 1) // 사물함에 물건 넣기
	{
		// 3. 원하는 사물함번호 입력받기
		sprintf(message, "\n사용하기를 원하는 사물함의 번호를 입력하세요 : ");
		send(cfd, &message, sizeof(message), 0);
		read(cfd, &locker_num, sizeof(locker_num));

		locker_use = locker[locker_num].use;
		locker_conn = locker[locker_num].conn;
		write(cfd, &locker_use, sizeof(locker_use));
		write(cfd, &locker_conn, sizeof(locker_conn));

		// 4. 사물함이 비어있는지, 연결중이 아닌지 확인
		if ((locker_use == 0) && (locker_conn == 0))
		{
			char ans;
			char client_pw[10], client_cpw[10];
			
			locker[locker_num].conn = 1;
			sprintf(message, "[%d]번 사물함이 비었습니다. 사용하시겠습니까?(Y/N) ", locker_num);
			send(cfd, &message, sizeof(message), 0);
			read(cfd, &ans, sizeof(ans));
			 
			if (ans == 'Y')
			{		 	
			 	locker[locker_num].use = 1;
			 	empty_locker--;
	 	
	 			// 5. 비밀번호 설정
			 	sprintf(message, "설정할 비밀번호를 입력해주세요 (10글자 이내의 영어, 숫자): ");
			 	send(cfd, &message, sizeof(message), 0);
			 	recv(cfd, &client_pw, sizeof(client_pw), 0);
			 	
			 	sprintf(message, "다시 입력해주세요: ");
			 	send(cfd, &message, sizeof(message), 0);
			 	recv(cfd, &client_cpw, sizeof(client_cpw), 0);
			 	
			 	strcpy(locker[locker_num].pw, client_pw);
			 	locker[locker_num].conn = 0; // 다 사용했으니 연결 끊기
			 }	 
		}
		else if (locker_use == 1)
		{
			 sprintf(message, "비어있는 사물함을 선택해주세요.\n");
			 send(cfd, &message, sizeof(message), 0);
		}
		else if (locker_conn == 1)
		{
			sprintf(message, "다른 사용자가 해당 사물함에 접근하고 있습니다. 잠시 후에 시도해주세요.\n");
			 send(cfd, &message, sizeof(message), 0);
		}
	}
	else if ((action_num == 2) && (locker_conn == 0)) // 사물함에서 물건 찾기
	{	
		int check;
		char client_pw[10]; 

		sprintf(message,"사물함 번호를 입력하세요: ");
		send(cfd, &message, sizeof(message), 0);
		locker[locker_num].conn = 1;

		recv(cfd, &locker_num, sizeof(locker_num), 0);
		locker_use = locker[locker_num].use;
		send(cfd, &locker_use, sizeof(locker_use), 0);
		locker_lock = locker[locker_num].lock;
		send(cfd, &locker_lock, sizeof(locker_lock), 0);
		
		if (locker_lock == 1)
		{
			sprintf(message,"사물함이 잠겼습니다. %d초 뒤에 다시 시도해주세요.\n", t);
     			send(cfd, &message, sizeof(message), 0);
     			locker[locker_num].conn = 0;
		}

		else if (locker_use == 0)
		{
			 sprintf(message, "비어있는 사물함입니다.\n본인이 사용중인 사물함을 선택해주세요.\n");
			 send(cfd, &message, sizeof(message), 0);
			 locker[locker_num].conn = 0;
		}
		else if (locker_use == 1)
		{
			 // 비밀번호를 입력하세요
		 	 send(cfd, &locker[locker_num].pw, sizeof(locker[locker_num].pw),0);
			 sprintf(message, "[1번째 시도] 비밀번호를 입력하세요: ");
			 send(cfd, &message, sizeof(message), 0);

			 // recv 비밀번호
			 recv(cfd, &client_pw, sizeof(client_pw), 0);
			 
			 int i=1;

			 while(i < 6)
			 {
				strcpy(locker_pw, locker[locker_num].pw);

				if(!strcmp(client_pw, locker_pw))
				{
				        locker[locker_num].use = 0;
				        locker[locker_num].conn = 0;
				        i=6;
					empty_locker++;
			    	}
				else
				{     				  
				     	if (i ==5)
				     	{
				     		int num = locker_num;
						locker[locker_num].lock = 1;
						locker[locker_num].conn = 0;
			     			signal(SIGALRM, alarmHandler);
				     		alarm(2); sleep(62);
				     		locker[num].lock = 0;
			      		}
					else
			     		{
						sprintf(message, "[%d번째 시도] 비밀번호를 다시 입력하세요: ",i+1);
						send(cfd, &message, sizeof(message), 0);
						recv(cfd, &client_pw, sizeof(client_pw), 0);
			     		}
				}
				i++;
			}
		}
	}

	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(cfd==clnt_socks[i])
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(cfd);
}


int main()
{
	int sfd, cfd, clientlen;
	struct sockaddr_un serverUNIXaddr, clientUNIXaddr;
	pthread_t p_thread;
	
	do {
		printf("원하는 사물함의 개수를 입력하세요(1~50) : ");
		scanf("%d", &locker_size);
		empty_locker = locker_size;
	
	} while(locker_size > MAX_LOCKER_SIZE || locker_size < 1);
	locker = (struct locker *) calloc(locker_size+1, sizeof(struct locker));		

	pthread_mutex_init(&mutx, NULL);
	
	sfd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
	serverUNIXaddr.sun_family = AF_UNIX;
	strcpy(serverUNIXaddr.sun_path, "convert");
	unlink("convert");
	bind(sfd, (struct sockaddr *)&serverUNIXaddr, sizeof(serverUNIXaddr));
	listen(sfd, 5);
	
	while(1){
		clientlen = sizeof(clientUNIXaddr);
		cfd = accept(sfd, (struct sockaddr *)&clientUNIXaddr, &clientlen);
		
		
		// 한 thread에는 한 명의 client만 접근
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = cfd;
		pthread_mutex_unlock(&mutx);
		
		pthread_create(&p_thread, NULL, t_function, (void *)&cfd);
		pthread_detach(p_thread);
		printf("client connect: %d \n", clnt_cnt);
		sleep(3);
		system("clear");
	}
   
	close(sfd);
	return 0;
}
