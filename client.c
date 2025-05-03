// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define PORT 5000
void handleError(char *error)
{
	printf("\033[0;31m%s\n\033[0m ", error);
	exit(1);
}
int main(int argc, char const *argv[])
{
	int sock = 0, valred;
	struct sockaddr_in serv_addr;

	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		handleError("\n Socket creation error \n");
		//return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	int ipvv = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	if(ipvv<=0)
	{
		handleError("\nInvalid address/ Address not supported \n");
		//return -1;
	}
	int connt=connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (connt< 0)
	{
		handleError("\nConnection Failed \n");
		//return -1;
	}
	printf("\033[0;33mEnter your query.\033[0m\n");
	while (1){
		char query[1024];
		bzero(query,1024);
		fgets(query, 1024, stdin);
		if(strlen(query)<=1){
			printf("\033[0;31mThe query was empty enter again.\033[0m \n");
			continue;
		}
		send(sock , query , strlen(query) ,0);
		printf("\033[0;33mSending\033[0m %s \n",query);
		bzero(buffer,1024);
		valred = recv(sock,buffer, 1024,0);
		printf("%s \033[0;33m--- recieved\033[0m\n",buffer );
		bzero(buffer,1024);
	}
	
	return 0;
}
