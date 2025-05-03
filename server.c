// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 5000
int N = 0; // store the count of number of lines
char *buf = NULL;
// Handle error in various phase
void handleError(char *error)
{
	printf("\033[0;31m%s\n\033[0m ", error);
	exit(1);
}

// Utility function to read line
char *readline(int linenumber)
{

	// handle negative number case
	if (linenumber < 0)
	{
		linenumber = N + linenumber;
	}
	// handle  index out of range of lines
	if (linenumber < 0 || linenumber > N - 1)
	{
		if (buf == NULL)
			buf = (char *)malloc(100 * sizeof(char));
		sprintf(buf, "The doc contains %d lines. Query exceeding Limit", N);
		return buf;
	}
	FILE *fp = fopen("server_file.txt", "r");
	char *line = NULL;
	size_t len, read, currentline = 0;
	fseek(fp, 0, SEEK_SET);
	// read till we encounter nth line
	while (((read = getline(&line, &len, fp)) != -1) && currentline < linenumber)
	{
		free(line);
		line = NULL;
		currentline++;
	}

	fclose(fp);
	return line;
}

// utility function to write at a particular line
int writeline(int linenumber, char *line)
{
	// handle negative number
	if (linenumber < 0)
	{
		linenumber = N + linenumber;
	}
	// if there is noting to write  => return
	if (strlen(line) == 0)
		return 0;
	// handle index out of range
	if (linenumber < 0 || linenumber > N)
	{
		char buf[100];
		sprintf(buf, "The doc contains %d lines. Query exceeding Limit", N);
		return -1;
	}
	N++;
	char *temp = NULL;
	FILE *fp = fopen("server_file.txt", "r");
	FILE *fpt = fopen("temp", "w");
	size_t len, read, currentline = 0;
	rewind(fp);
	// read till N lines
	while (((read = getline(&temp, &len, fp)) != -1) && currentline < linenumber)
	{
		fprintf(fpt, "%s", temp);
		free(temp);
		temp = NULL;
		currentline++;
	}
	// write the nth line
	fprintf(fpt, "%s\n", line);
	// write n+1 th line to last line
	while (read != -1)
	{
		fprintf(fpt, "%s", temp);
		free(temp);
		temp = NULL;
		currentline++;
		read = getline(&temp, &len, fp);
	}
	fclose(fp);
	fclose(fpt);
	// replace the file with modified file file
	system("rm -rf server_file.txt && mv temp server_file.txt ");

	return 0;
}

// utility function to match pattern
int match(char *str, char *pattern, int pattern_length)
{
	for (int i = 0; i < pattern_length; i++)
	{
		if (str[i] != pattern[i])
			return 0;
	}
	return 1;
}
// handle query
void handleQuery(char *query)
{
	// parse the query
	int line = 5555;
	char s[1024], s1[1024], sm[1024], errorcheck[1024];
	bzero(s1, 1024);
	bzero(errorcheck, 1024);
	strcpy(errorcheck, query);
	sscanf(query, "%[^ -0123456789]%d%[ \t]%[^\n]", s, &line, sm, s1);
	//sscanf(query,"%[^ -0123456789]%s%[ \t]%[^\n]",s,&line,sm,s1);
	bzero(query, 1024);
	// compare with input format
	if (strncmp(s, "NLINEX",6) == 0)
	{
		char tempp[1024], tt[1024];
			bzero(tempp, 1024);
			sscanf(errorcheck, "NLINEX%[ ]%s", tt, tempp);
			if (strlen(tempp) != 0)
			{

				sprintf(query, "\033[0;31mError in argument\n\033[0m");
				printf("\033[0;31mError in argument\n\033[0m");
				return;
			}

		sprintf(query, "%d\n", N);
		//TODO: implement no of lines
	}
	else if (strncmp(s, "READX", 5) == 0)
	{
		if (line == 5555)
		{
			char tempp[1024], tt[1024];
			bzero(tempp, 1024);
			sscanf(errorcheck, "READX%[ ]%s", tt, tempp);
			if (strlen(tempp) != 0)
			{

				sprintf(query, "\033[0;31mError in argument\n\033[0m");
				printf("\033[0;31mError in argument\n\033[0m");
				return;
			}
			line = 0;
		}
		if (strlen(s1) > 0){
			sprintf(query, "\033[0;31mError in argument\n\033[0m");
				printf("\033[0;31mError in argument\n\033[0m");
				return;
		}

		sprintf(query, "%s", readline(line));
		printf("\033[0;32mRead successful\n\033[0m");

		//TODO: read  line
	}
	else if (strncmp(s, "INSERTX", 7) == 0)
	{
		if (line == 5555)
		{
			char tempp[1024], tt[1024];
			bzero(tempp, 1024);
			sscanf(errorcheck, "INSERTX%[ ]%s", tt, tempp);
			if (strlen(tempp) == 0)
			{
				
				sprintf(query, "\033[0;31mError in argument\n\033[0m");
				printf("\033[0;31mError in argument\n\033[0m");
				return;
			}else{
				line=N;
				writeline(line, tempp);
		sprintf(query, "\033[0;32mwrite successful\n\033[0m");
		printf("\033[0;32mwrite successful\n\033[0m");
		return;
			}
		}
		
		if (line > N)
		{
			sprintf(query, "\033[0;31mwrite unsucessfull Exceding document limit\n\033[0m");
			return;
		}
		writeline(line, &s1[0]);
		sprintf(query, "\033[0;32mwrite successful\n\033[0m");
		printf("\033[0;32mwrite successful\n\033[0m");
	}
	else
	{
		// handle invalid command
		sprintf(query, "\033[0;31m Invalid Command\n\033[0m");
	}
}

int main(int argc, char const *argv[])
{
	FILE *fptr = fopen("server_file.txt", "r");
	rewind(fptr);

	if (fptr == NULL)
	{
		printf("\033[0;31mcould not read the file\033[0m\n");
		handleError("default file dont exist");
	}
	fclose(fptr);
	fptr = fopen("server_file.txt", "a+");
	char c;
	fseek(fptr, -1 * sizeof(char), SEEK_END);

	if (c = fgetc(fptr) != '\n')
	{
		// printf("adding a new line\n");
		fprintf(fptr, "\n");
	}
	// getc(fptr);
	rewind(fptr);
	//push(vec,ftell(fptr));
	char *line = NULL;
	size_t len, read, currentline = 0;

	read = getline(&line, &len, fptr);
	while (read != -1)
	{
		N++;
		read = getline(&line, &len, fptr);
	}

	fclose(fptr);
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		handleError("socket failed");
	}
	// Forcefully attaching socket to the port 5000
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				   &opt, sizeof(opt)))
	{
		handleError("setsockopt");
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
			 sizeof(address)) < 0)
	{
		handleError("bind failed");
		//exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 1) < 0)
	{
		handleError("listen");
		//exit(EXIT_FAILURE);
	}
	printf("\033[0;32mServer started Waiting for client\n\033[0m");
	printf("Type \033[0;31mCtrl/Cmd+C \033[0m to quit\n");
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
							 (socklen_t *)&addrlen)) < 0)
	{
		handleError("accept");
		//exit(EXIT_FAILURE);
	}
	printf("\033[0;33mClient connected. Waiting for client to query.\n\033[0m");
	while (1)
	{

		bzero(buffer, 1024);
		valread = recv(new_socket, buffer, 1024, 0); // waiting to recieve messages
		if (valread < 0)
		{
			handleError("error on reading from client\n");
		}
		printf("\033[0;33mRequested query is\033[0m %s\n", buffer); // read the message

		handleQuery(buffer);							 // handle message
		printf("\033[0;33mSending\033[0m %s\n", buffer); // send the response
		send(new_socket, buffer, strlen(buffer), 0);
		bzero(buffer, 1024);
	}

	return 0;
}