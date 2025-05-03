#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sys/sendfile.h>
#include <sys/shm.h>
#include <sys/wait.h>
using namespace std;
#define PORT 5000

/**
* Shubham Soni(21CS60R09)
* compiler : g++ client.cpp -o client
* run command : ./client
* operating system : linux
**/

int Clientid=0;

//utility class for logging file
class Log
{
private:
    int priority = 1;
    //bool enable = true;
public:
    Log()
    {
    }
    void d(const std::string &s)
    {
        if (priority <= 0)
        {
            std::cout << s << std::endl;
        }
    }
    void waring(const std::string &s)
    {
        if (priority <= 1)
        {
            std::cout << "\033[0;33m"<< s<< "\033[0m" << std::endl;
        }
    }
    void error(const std::string &s)
    {
        if (priority <= 2)
        {
            std::cout << "\033[0;31m" << s << "\033[0m" << std::endl;
        }
    }
    void success(const std::string &s)
    {
        std::cout << "\033[0;32m" << s << "\033[0m"
                  << std::endl;
    }
    void wait(const std::string &s)
    {
        std::cout << "\033[0;33m"<< s<< "\033[0m" << std::endl;
    }
    void setLogLevel(int n)
    {
        // set log level 0: verbose 1:waring 2:only errors 3:disable
        if (n <= 3 && n >= 0)
            priority = n;
    }
};

Log lg;

void handleError(string error)
{
    printf("\033[0;31m%s\n\033[0m ", error.c_str());
    exit(1);
}

char f1[100] = {0};
char f2[100] = {0};

//utility function to send a file
bool sendFile(int sock, char *filename)
{
    int fin = open(filename, O_RDONLY);
    struct stat stats;
    fstat(fin, &stats);
    long tsize = 0;
    long size = stats.st_size;
    cout << "size transfered is " << size << "bytes" << endl;
    send(sock, &size, sizeof(long), 0);
    sendfile(sock, fin, NULL, size);
    char message[10] = {0};
    recv(sock, message, 10, 0);
    cout << "File is " << message << "\n";
    if (message[0] == 'F')
        return false;
    if (message[0] == 'O')
        lg.d(message);
    else
        return false;
    return true;
}

// utility function to check whether the file exit in the directory
bool checkFile(char *filename)
{
    FILE *fptr = fopen(filename, "r");

    if (fptr == NULL)
    {
        printf("\033[0;31mcould not read the file\033[0m\n");
        lg.error("default file dont exist\n");
        return false;
    }
    fclose(fptr);
    return true;
}

const string intvite_pat = R"(\/invite\s[a-zA-z0-9\_\-]+[\.]?\w*\s\d+\s[VE][\n]?)";
const string insert_pat = R"(\/insert\s[a-zA-z0-9\_\-]+[\.]?\w*\s([-]?\d*\s)?".*?"[\n]?)";
const string delete_pat = R"(\/delete\s[a-zA-z0-9\_\-]+[\.]?\w*\s?[-]?\d*\s?[-]?\d*[\s\n]?)";
const string read_pat = R"(\/read\s[a-zA-z0-9\_\-]+[\.]?\w*\s?[-]?\d*\s?[-]?\d*[\s\n]?)";
const string upload_pat = R"(\/upload\s[a-zA-z0-9\_\-]+(\.\w+)?[\s\n]?)";
const string download_pat = R"(\/download\s[a-zA-z0-9\_\-]+[.]?\w*[\s\n]?)";
const string user_patch = R"(\/users[\s\n]?)";
const string files_patch = R"(\/files[\s\n]?)";
#include <regex>

//utility function to validate query
bool validate(char *query)
{
    string q = query;

    char a[100] = {0, 0}, b[100] = {0, 0}, c[100] = {0, 0}, d[100] = {0, 0}, e[100] = {0, 0};
    sscanf(query, "/%[^ \t\n] %[^ \t\n] %[^ \t\n] %[^ \t\n] %[^ \t\n]", a, b, c, d, e);
    if (strncmp(a, "upload", 7) == 0)
    {
        if(regex_match(q, regex{upload_pat}) && checkFile(b)){
            return true;
        }else
        return false ;
    }
    else if (strncmp(a, "download", 8) == 0)
    {
        return regex_match(q, regex{download_pat});
    }
    else if (strncmp(a, "invite", 6) == 0)
    {
        return regex_match(q, regex{intvite_pat});
    }
    else if (strncmp(a, "users", 5) == 0)
    {
        return regex_match(q, regex{user_patch});
    }
    else if (strncmp(a, "files", 5) == 0)
    {
        return regex_match(q, regex{files_patch});
    }
    else if (strncmp(a, "read", 4) == 0)
    {
        return regex_match(q, regex{read_pat});
    }
    else if (strncmp(a, "delete", 6) == 0)
    {
        return regex_match(q, regex{delete_pat});
    }
    else if (strncmp(a, "insert", 6) == 0)
    {
        return regex_match(q, regex{insert_pat});
    }
    else if (strncmp(a, "exit", 4) == 0)
    {

        return true;
    }
    else if (query[0] == 'Y' || query[0] == 'N' || query[0] == 'y' || query[0] == 'n')
    {
        return true;
    }
    return false;
}

// utility  function recieve buffer
void recieveBuf(char *b, int sock)
{
    // cout << "At recieve buff \n";
    size_t ln;
    recv(sock, &ln, sizeof(size_t), 0);
    size_t i = 0;
    FILE *f;
    if (b != NULL)
        f = fopen(b, "w");
    char buf[500] = {0};
    bzero(buf, 500 * sizeof(char));
    while (i < ln)
    {
        recv(sock, buf, 500, 0);
        if (b != NULL)
            fprintf(f, "%s", buf);
        else
        {
            if (i % 2 == 0)
            {
                string st = buf;
                st.replace(st.length() - 1, 1, "\t | \t");
                cout << st;
            }
            else
            {
                printf("%s", buf);
            }
        }
        bzero(buf, 500 * sizeof(char));
        i++;
        send(sock, &i, sizeof(size_t), 0);

        /* code */
    }
    if (b != NULL)
        fclose(f);
    // cout << "Recieved Successful \n";
}
// utility function to recieve a file
bool recieveFile(const char *filename, int newsock)
{
    long sz = 0;
    recv(newsock, &sz, sizeof(sz), 0);
     std::ofstream file(filename, std::ios::binary);
    if(sz!=0) {
    char *content = new char[sz];
   
    long tsz = 0;
    
    while (sz != tsz)
    {
        long red = recv(newsock, content + tsz, sz - tsz, 0);
        tsz = red + tsz;
    } // read the file
    file.write(content, sz);
    delete[] content;
    }
    file.close();
    char message[10];
    bzero(message, 10);
    strcpy(message, "OK");
    send(newsock, message, 10, 0);
    return true;
}

void sendstr(const char *str, int newsock)
{
    char buf[256] = {0};
    bzero(buf, 256);
    strcpy(buf, str);
    send(newsock, buf, 256, 0);
}

void recievestr(char *str, int newsock, long size)
{
    char buf[size] = {0};
    bzero(str, size);
    bzero(buf, size);
    long tsz = 0;
   
    while (size != tsz)
    {
        long red = recv(newsock, buf + tsz, size - tsz, 0);
        tsz = red + tsz;
    }
    strcpy(str, buf);
}
// utility function to print a buffer file
void printFile(int sock){
            int pid = getpid();
            string t = to_string(pid);
            const char *temp = t.c_str();
            //lg.success(buf);
            recieveFile(temp, sock);
            ifstream f(temp);
            string toprint;
            while(getline(f,toprint))cout << toprint<<std::endl;
            f.close();
            remove(temp);
}

// utility  function to handle handle query
void handleQuery(char *query, int sock)
{
    // cout << "At handle Query \n";
    // parse the query
    int line = 5555;
    char s[1024], s1[1024], sm[1024], errorcheck[1024];
    char a[100], b[100], c[100], d[100], e[100];
    bzero(s1, 1024);
    bzero(s, 1024);
    bzero(errorcheck, 1024);
    strcpy(errorcheck, query);
    sscanf(query, "/%[^ \t\n] %[^ \t\n] %[^ \t\n] %[^ \t\n] %[^ \t\n]", s, b, c, d, e);

    string cmd = s;
    if (cmd == "users")
    {
        char buf[256] = {0};
        recievestr(buf, sock, 256);
        lg.success(buf);
    }
    else if (cmd == "download")
    {
        char buf[256] = {0};
        recievestr(buf, sock, 256);
        if (buf[0] == 'O')
        {
            lg.success(buf);
            char fnamee[100],ext[10]={0};
            bzero(ext,10);
            bzero(fnamee,100);
            sscanf(b,"%[^.].%s",fnamee,ext);
            char res[200];
            if(strlen(ext)==0){
                sprintf(res,"%s_%d",fnamee,Clientid);
            }else{
                sprintf(res,"%s_%d.%s",fnamee,Clientid,ext);
            }
            
            recieveFile(res, sock);
        }
        else
        {
            lg.error(buf);
        }
    }
    else if (cmd == "upload")
    {
        char name[256];
        sendstr(b, sock);
        recievestr(name, sock, 256);
        if (name[0] == 'O')
        {
            sendFile(sock, b);
            lg.success("Uploaded successFully");
        }
        else
        {
            lg.error(name);
        }
    }
    else if (cmd == "read"||cmd=="files")
    {
        char buf[256] = {0};
        recievestr(buf, sock, 256);
        if (strncmp(buf, "OK", 2) == 0)
        {
            printFile(sock);
        }
        else
        {
            lg.error(buf);
        }
    }
    else if (cmd == "delete" || cmd == "insert" || cmd == "invite")
    {
        char buf[256] = {0};
        recievestr(buf, sock, 256);
        if (buf[0] == 'O')
        {
            lg.success(buf);
           
            if(cmd != "invite"){
                lg.waring("The changed content is as follows");
                printFile(sock);
            }
        }
        else
        {
            lg.error(buf);
        }
    }
    else if (cmd == "check\n")
    {
        char buf[256] = {0};
        recievestr(buf, sock, 256);
        if (buf[0] == 'Y')
        {
            lg.success(buf);
            sendstr("Yes", sock);
        }
        else
        {
            lg.error(buf);
        }
    }
    else if (query[0] == 'Y' || query[0] == 'N' || query[0] == 'y' || query[0] == 'n')
    {
        char buf[256] = {0};
        recievestr(buf, sock, 256);
        lg.success(buf);
    }
}
int main(int argc, char const *argv[])
{
    int loop = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT);
    int req = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT);
    int *lp = (int *)shmat(loop, 0, 0);
    int *reqq = (int *)shmat(req, 0, 0);
    *reqq = 0;
    *lp = 0;
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
    if (ipvv <= 0)
    {
        handleError("\nInvalid address/ Address not supported \n");
        //return -1;
    }
    int connt = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (connt < 0)
    {
        handleError("\nConnection Failed \n");
        //return -1;
    }
    bzero(buffer, 1024);
    recv(sock, buffer, 1024, 0);
    printf("%s", buffer);
    if (buffer[0] != 'W')
    {
        //printf("%s",buffer);
        exit(1);
    }
    char xyz[70];
    sscanf(buffer,"%[^0123456789]%d",xyz,&Clientid);
    bzero(buffer, 1024);
    printf("\n");
    printf("\033[0;33mEnter your query.\033[0m\n");
    char command[100], filename1[100], filename2[100];

    char query[1024];
    while (1)
    {
        *lp = 1;
        int cid;
        if ((cid = fork()) == 0)
        {
            while (1)
            {

            char buf[256] = {0};
            recievestr(buf, sock, 256);
            if (buf[0] == 'Y')
            {
                lg.success(buf);
                *reqq = 1;
                exit(0);
            }
            else if(buf[0]=='U'){
                lg.wait(buf);
                continue;
            }
            else
            {
                exit(0);
                
            }

            exit(0);
            }
        }

        bzero(query, 1024);
        fgets(query, 1024, stdin);
        *lp = 0;

        if (*reqq == 1)
        {
            while (!(query[0] == 'Y' || query[0] == 'N' || query[0] == 'y' || query[0] == 'n'))
            {
                
                lg.error("Please answer you query in yes(Y) or no(N)");
                bzero(query, 1024);
                fgets(query, 1024, stdin);
            }
            
            *reqq = 0;
        }

        while (strlen(query) <= 1)
        {
            printf("\033[0;31mThe query was empty enter again.\033[0m \n");
            bzero(query, 1024);
            fgets(query, 1024, stdin);
        }

        while (!validate(query))
        {
            lg.error("Wrong query entered or file is missing or invalid file");
            printf("\033[0;33mEnter your query.\033[0m\n");
            bzero(query, 1024);
            fgets(query, 1024, stdin);
        }

        send(sock, query, strlen(query), 0);
        // if (strncmp(query, "/sort", 5) == 0)
        // {
        //     sscanf(query, "/sort %[^ \t\n]", f1);
        // }
        if (strncmp(query, "/exit", 5) == 0)
        {
            bzero(query, 1024);

            int wid = waitpid(cid, NULL, WNOHANG);
            if (cid == wid)
            {
                cout << "exited succesfully";
            }
            close(sock);
            printf("[-]Disconnected from server.\n");
            kill(cid, 0);
            exit(1);
        }
        printf("\033[0;33mSending\033[0m %s \n", query);
        // bzero(buffer, 1024);
        //valred = recv(sock, buffer, 1024, 0);
        int wid = waitpid(cid, NULL, 0);
        if (cid == wid)
        {
            lg.waring("Processing your query\n");
        }
        handleQuery(query, sock);
        printf("%s \033[0;33m--- recieved\033[0m\n", buffer);
        bzero(buffer, 1024);
    }

    return 0;
}