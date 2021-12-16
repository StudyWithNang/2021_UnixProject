#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/socket.h>
#include "locker.h"
#define DEFAULT_PROTOCOL 0
#define MAXLINE 100
;

int locker_num, locker_use, locker_conn, locker_lock;
char client_pw[10], locker_pw[10];

void server_recv_message(int sfd)
{
	char message[800];
	recv(sfd, &message, sizeof(message), 0);
	printf("%s\n", message);
}

int main()
{
	int sfd, result;
	int action_num;
	char message[200];
	char welcome[400];
	struct sockaddr_un serverUNIXaddr;

	sfd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
	serverUNIXaddr.sun_family = AF_UNIX;
	strcpy(serverUNIXaddr.sun_path, "convert");

	do { 
		result = connect(sfd, (struct sockaddr *)&serverUNIXaddr, sizeof(serverUNIXaddr));
		if (result == -1) sleep(1);
	} while (result == -1);

	
	// 1. welcome + action_num 받기
	recv(sfd, &welcome, sizeof(welcome), 0);
	printf("%s", welcome);
	
	fflush(stdout);
		
	scanf("%d", &action_num);
	write(sfd, &action_num, sizeof(action_num));
	
	// 2. 현재 사물함의 정보 알려줌
	server_recv_message(sfd);

	if (action_num == 1)
	{
		// 3. 원하는 사물함번호 입력받기		
		recv(sfd, &message, sizeof(message), 0);
		printf("%s", message);
		scanf("%d", &locker_num);
		write(sfd, &locker_num, sizeof(locker_num));
		
		read(sfd, &locker_use, sizeof(locker_use));
		read(sfd, &locker_conn, sizeof(locker_conn));

		// 4. 사물함이 비어있는지, 연결중이 아닌지 확인
		if ((locker_use == 0) && (locker_conn == 0))
		{
			char ans;
			char client_pw[10], client_cpw[10];
			
			recv(sfd, &message, sizeof(message), 0);
			printf("%s", message);
			scanf(" %c", &ans);
			printf("\n");
			write(sfd, &ans, sizeof(ans));
			
			
			if (ans == 'Y')
			{
				// 5. 비밀번호 설정
				// 설정할 비밀번호를 입력해주세요
				recv(sfd, &message, sizeof(message), 0);
				printf("%s", message);
				scanf(" %s", client_pw);
				send(sfd, &client_pw, sizeof(client_pw), 0);
				 
				// 다시 입력해주세요		 
				recv(sfd, &message, sizeof(message), 0);

				do{ 
					printf("%s", message);
					scanf(" %s", client_cpw);
				
				} while(strcmp(client_pw, client_cpw));
				 
				send(sfd, &client_cpw, sizeof(client_cpw), 0);
				printf("\n감사합니다. 안녕히가세요 :)\n");
				 
			}
			else if (ans == 'N')
			{
				printf("\n안녕히가세요 :)\n");
			}
		}
		else if (locker_use == 1)
		{
			 recv(sfd, &message, sizeof(message), 0);
			 printf("%s", message);
		}
		else if (locker_conn == 1)
		{
			recv(sfd, &message, sizeof(message), 0);
			printf("%s", message);
		}
	
	}

	else if (action_num == 2)
	{
		int check;

		// 사물함 번호를 입력하세요
		recv(sfd, &message, sizeof(message), 0);
		printf("%s", message);
		scanf("%d", &locker_num);
		printf("\n");
		send(sfd, &locker_num, sizeof(locker_num), 0);

		// locker 사용 여부 판단 (0: empty, 1: full)
		recv(sfd, &locker_use, sizeof(locker_use), 0);
		recv(sfd, &locker_lock, sizeof(locker_lock), 0);
		
		if (locker_lock == 1)
		{
     			recv(sfd, &message, sizeof(message), 0);
     			printf("%s", message);
		}

		else if (locker_use == 0) // empty
		{
			recv(sfd, &message, sizeof(message), 0);
			printf("%s\n", message);
		}
		else if (locker_use == 1)
		{
			// 비밀번호를 입력하세요
			recv(sfd, &locker_pw, sizeof(locker_pw),0);
			recv(sfd, &message, sizeof(message), 0);
			printf("%s", message);
			scanf("%s", client_pw);
			send(sfd, &client_pw, sizeof(client_pw), 0);

			// 비밀번호를 다시 입력하세요
			int i = 1;
			while(i < 6)
			{
				check = strcmp(client_pw, locker_pw);

				if (!strcmp(client_pw, locker_pw))
				{
					printf("\n사용해주셔서 감사합니다\n");
					i=6;
					exit(0);
				}
				else
				{
					if(i ==5)
					{
						printf("잘못된 비밀번호입니다. 1분 동안 [%d]번 사물함이 잠깁니다.\n",locker_num);
						exit(0);
					}
					else
					{
						//pw again
						recv(sfd, &message, sizeof(message), 0);
						printf("%s", message);
						scanf("%s", client_pw);
						send(sfd, &client_pw, sizeof(client_pw), 0);
					}

				}
				i++;
			}
		}
		
	}
	else printf("\n안녕히가세요 :)\n");

	close(sfd);
	exit(0);
}