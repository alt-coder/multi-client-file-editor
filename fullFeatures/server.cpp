#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sstream>
#include <regex>
#include <iomanip>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#define PORT 5000
#define NClients 4 // maximum number of client that can be connected
int current_client = 0;

/**
 * Shubham Soni(21CS60R09)
 * compiler : g++ server.cpp -o server
 * run command : ./server
 * operating system : linux
 **/
template <typename T>
class Node
{

public:
    T data;
    Node *next = nullptr;
    Node()
    {
    }
    // Node():next{NULL}{}
    Node(T data) : data{data}
    {
    }
};
template <typename T>
class List
{
private:
    Node<T> *head = new Node<T>();

    int size = 0;
    Node<T> *lastnode = nullptr;

    class iterator
    {
    public:
        iterator(Node<T> *ptr) : ptr(ptr) {}
        iterator operator++()
        {
            ptr = ptr->next;
            return *this;
        }
        bool operator!=(const iterator &other) const { return ptr != other.ptr; }
        T &operator*() const { return ptr->data; }

    private:
        Node<T> *ptr;
    };

public:
    int length() { return size; }
    List() {}
    void pushback(T obj)
    {
        Node<T> *n = new Node<T>(obj);
        if (head->next == nullptr)
        {
            head->next = n;

            // head.hasdata=1;
        }
        else
        {
            lastnode->next = n;
            // lastnode.hasdata=1;
        }
        lastnode = n;
        size++;
    }

    void print()
    {
        T *a;
        int count = 0;
        Node<T> *temp = head->next;
        while (temp != nullptr)
        {
            std::cout << temp->data << "->";
            a = &(temp->data);
            temp = temp->next;
        }
    }
    // void pushback(){
    //     std::cout<<"i also get called"<<"\n";
    // }
    iterator begin() const { return iterator(head->next); }
    iterator end() const { return iterator(nullptr); }
    template <typename... Types>
    void pushback(T var, Types... var2)
    {
        pushback(var);
        pushback(var2...);
    }
    T &operator[](int index)
    {
        auto itr = this->begin();
        int l = static_cast<int>(size);
        if (index < 0 && index >= l * -1)
        {
            int actual_index = l + index;
            return this->operator[](actual_index);
        }
        if (index >= 0 && index < l)
        {
            for (int t = 0; t < index && itr != this->end(); t++)
            {
                ++itr;
            }
        }
        else
        {
            throw std::invalid_argument("Received invaild value");
        }
        return *itr;
    }

    const T &operator[](int index) const
    {
        auto itr = this->begin();
        int l = static_cast<int>(size);
        if (index < 0 && index >= l * -1)
        {
            int actual_index = l + index;
            return this->operator[](actual_index);
        }
        if (index < l)
        {
            for (int t = 0; t < index && itr != this->end(); t++)
            {
                ++itr;
            }
        }
        else
        {
            throw std::invalid_argument("Received invaild value");
        }
        return *itr;
    }
    void pop()
    {
        auto node = head->next;
        // Node<T> * t;
        if (node == nullptr)
        {
            return;
        }
        else
        {
            if (node == lastnode)
            {
                delete node;
                lastnode = nullptr;
                head->next = nullptr;
                size--;
                return;
            }
            else
            {
                while (node->next != lastnode)
                {
                    node = node->next;
                }
                delete lastnode;
                lastnode = node;
                lastnode->next = nullptr;
                size--;
            }
        }
    }
    void remove(int index)
    {
        int l = static_cast<int>(size);
        if (index < l)
        {
            Node<T> *before = head;
            for (int i = 0; i < index; i++)
            {
                before = before->next;
            }
            if (before->next == lastnode)
            {
                delete lastnode;
                lastnode = before;
                lastnode->next = nullptr;
                size--;
            }
            else if (before->next != nullptr)
            {
                Node<T> *temp = before->next;
                before->next = before->next->next;
                delete temp;
                size--;
            }
        }
    }

    ~List()
    {
        // std::cout << "distructor is called\n";
        Node<T> *temp = head->next;
        while (temp != nullptr)
        {
            Node<T> *n = temp->next;
            delete temp;
            temp = n;
        }
    }
};
const std::string pat = R"([0-9][0-9]\.[0-9][0-9]\.[0-9][0-9][0-9][0-9]\t[A-Za-z0-9_\s].*?\t\d.*?\.\d\d?[\s]?)";
std::regex pattern{pat};

// class for logging messaages
class Log
{
private:
    int priority = 1;
    // bool enable = true;
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
            std::cout << s << std::endl;
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
                  << "\n";
    }
    void wait(const std::string &s)
    {
        std::cout << "\033[0;33m" << s << "\033[0m"
                  << "\n";
    }
    void setLogLevel(int n)
    {
        // set log level 0: verbose 1:waring 2:only errors 3:disable
        if (n <= 3 && n >= 0)
            priority = n;
    }
};

Log lg;

// handle error
void handleError(const std::string &error)
{
    std::cout << "\033[0;31m" << error << "\n\033[0m ";
    // printf(, error.c_str());
    exit(1);
}
// Function to recieve file
bool recieveFile(const char *filename, int newsock)
{
    long sz = 0;
    recv(newsock, &sz, sizeof(sz), 0);
    char *content = new char[sz];
    std::ofstream file(filename, std::ios::binary);
    long tsz = 0;
    // std::cout << read(newsock, content, sz) << std::endl;
    //  long exp = read(newsock, content, sz);
    while (sz != tsz)
    {
        long red = recv(newsock, content + tsz, sz - tsz, 0);
        tsz = red + tsz;
    } // read the file
    file.write(content, sz);
    file.close();
    delete[] content;
    char message[10];
    bzero(message, 10);
    strcpy(message, "OK");
    send(newsock, message, 10, 0);
    return true;
}

// Function to send file
bool sendFile(int sock, const char *filename)
{

    int fin = open(filename, O_RDONLY);
    struct stat stats;
    fstat(fin, &stats);
    long tsize = 0;
    long size = stats.st_size;
    std::cout << "size transferred is " << size << " bytes" << std::endl;

    send(sock, &size, sizeof(long), 0);
    if (size != 0)
        sendfile(sock, fin, NULL, size);
    char message[10] = {0};
    recv(sock, message, 10, 0);
    std::cout << "File is " << message << "\n";
    if (message[0] == 'F')
        return false;
    if (message[0] == 'O')
        lg.d(message);
    else
        return false;
    return true;
}

int users[NClients];
// funtion to send string
void sendstr(const char *str, int newsock)
{
    char buf[256] = {0};
    bzero(buf, 256);
    strcpy(buf, str);
    send(newsock, buf, 256, 0);
}
// function to recieve string
void recievestr(char *str, int newsock, long size)
{
    char buf[size] = {0};
    bzero(buf, size);
    long tsz = 0;
    bzero(str, size);

    while (size != tsz)
    {
        long red = recv(newsock, buf + tsz, size - tsz, 0);
        tsz = red + tsz;
    }
    strcpy(str, buf);
}

// utility function to send files and records to client
void sendbuf(char *f, size_t n, int newsock)
{
    // std::cout << "At send buf \n";
    char buf[500] = {0};
    bzero(buf, 500);
    size_t ln = n;
    send(newsock, &ln, sizeof(size_t), 0); // send the number of lines  to be send
    for (size_t i = 0; i < n;)
    {
        auto rec = f[i];
        strcpy(buf, "rec");
        send(newsock, buf, 500, 0); // send lines one by one
        bzero(buf, 500);
        recv(newsock, &i, sizeof(i), 0); // update the counter in sync with client
    }
    // std::cout << "buffer sent succesfully \n";
}

struct File
{
    std::string filename;
    int m_owner;
    int collaborator[NClients];
    char permission[NClients];
    int lines = 0;
    File(int owner)
    {
        for (int i = 0; i < NClients; i++)
            collaborator[i] = permission[i] = 0;
        m_owner = owner;
    }
    File(int owner, const char *fname, int nline)
    {
        // bzero(filename,50);
        for (int i = 0; i < NClients; i++)
            collaborator[i] = permission[i] = 0;
        m_owner = owner;
        filename = fname;
        lines = nline;
    }
    File()
    {
        for (int i = 0; i < NClients; i++)
            collaborator[i] = permission[i] = 0;
    }
    void setOwner(int i)
    {
        m_owner = i;
    }
    void setColab(int colab, char P)
    {
        int i = 0;
        while (i < NClients)
        {
            if (collaborator[i] == 0 || collaborator[i] == colab)
            {
                collaborator[i] = colab;
                permission[i] = P;
                break;
            }
            i++;
        }
    }
    void removeColab(int colab)
    {
        int i = 0;
        while (i < NClients)
        {
            if (collaborator[i] == colab)
            {
                collaborator[i] = 0;
                permission[i] = 0;
                break;
            }
            i++;
        }
    }
    std::string tostring()
    {
        std::ostringstream ott;
        ott << "{ \n"
            << "\t\"file_name\": " << "\""<< filename<<"\"" << ",\n";
        ott << "\t\"owner id\":" << 10000 + m_owner << ",\n";
        ott << "\t\"collaborators\": \n\t\t[ \n";
        for (int i = 0; i < NClients; i++)
        {
            if (collaborator[i] != 0)
            {
                ott << "\t\t{ " << collaborator[i] + 10000 << " : " << permission[i] << " },\n";
            }
        }

        ott << "\t\t],\n";
        ott << "\t\"lines\":" << lines << "\n}";
        return ott.str();
    }
};

List<File> dir; // linked list to maintain records
// write a json file
void writeFile()
{
    std::ofstream f{"server_records.json", std::ios::out};
    f << "{\"records\":[\n";
    for (int i = 0; i < dir.length(); i++)
    {
        f << dir[i].tostring() ;
        if(i !=dir.length()-1) f<<",\n";
        else f<<"\n";
    }
    f << "]\n}";
    f.close();
}

char *buff;
// struct to maintain invites
struct Invite
{
    File *file = NULL;
    int to_invite;
    char P;
    int from;
    /* data */
};

List<Invite> invites;
File *curr_file;
// check permission
char checkPermission(const char *b, int newsock)
{
    bool toSend = false;
    for (int i = 0; i < dir.length(); i++)
    {
        if (dir[i].filename == b)
        {
            if (dir[i].m_owner == newsock)
            {
                curr_file = &dir[i];
                return 'E';
            }

            for (int j = 0; j < NClients; j++)
            {
                toSend = dir[i].collaborator[j] == newsock;
                if (toSend)
                {
                    curr_file = &dir[i];
                    return dir[i].permission[j];
                };
            }
        }
        if (toSend)
            break;
    }
    curr_file = NULL;
    return 0;
}
// calculate total number of lines
int NLine(const char *fname)
{
    int n = 0;
    std::string line;
    std::ifstream file(fname, std::ios::in);
    if (file.is_open())
    {
        while (getline(file, line))
        {
            n++;
            /* code */
        }
    }
    else
        return -1;
    file.close();
    return n;
}
// read from file
void readline(const char *filename, int linenumber, int endline, int N, int newsock)
{
    std::ifstream file(filename, std::ios::in);

    if (linenumber < 0)
        linenumber = N + linenumber;
    if (endline < 0)
        endline = N + endline;
    if (endline < 0 || endline >= N || linenumber > endline || linenumber < 0 || linenumber > N || N <= 0)
    {
        sendstr("Fail Wrong query", newsock);
        return;
    }
    std::fstream temp("temp", std::ios::out);
    if (file.is_open())
    {
        std::string st;
        int i = 0;
        while (getline(file, st))
        {
            if (i <= endline && i >= linenumber)
            {
                temp << st << "\n";
            }
            i++;
        }
    }
    else
    {
        sendstr("Fail Could not open file at server\n", newsock);
        return;
    }

    sendstr("OK", newsock);
    temp.close();
    sendFile(newsock, "temp");
    remove("temp");
    file.close();
}
// delete from file
bool del(const char *filename, int linenumber, int endline, int N, int newsock)
{
    std::ifstream file(filename, std::ios::in);
    std::fstream temp("temp", std::ios::out);
    if (linenumber < 0)
        linenumber = N + linenumber;
    if (endline < 0)
        endline = N + endline;
    if (endline < 0 || endline >= N || linenumber > endline || linenumber < 0 || linenumber > N || N <= 0)
    {
        sendstr("Fail", newsock);
        return false;
    }

    if (file.is_open())
    {
        std::string st;
        int i = 0;
        while (getline(file, st))
        {
            if (i <= endline && i >= linenumber)
            {
                i++;
                continue;
            }
            else
                temp << st << "\n";
            i++;
        }
    }
    else
    {
        sendstr("Fail", newsock);
        return false;
    }
    sendstr("OK Sucessfully deleted", newsock);
    file.close();
    temp.close();
    remove(filename);
    rename("temp", filename);
    sendFile(newsock, filename);
    return true;
}
// insert to a file
bool insert(const char *filename, int linenumber, std::string content, int N, int newsock)
{
    std::ifstream file(filename, std::ios::in);
    std::fstream temp("temp", std::ios::out);
    if (linenumber < 0)
        linenumber = N + linenumber;
    if (linenumber < 0 || linenumber > N || N < 0)
    {
        sendstr("Fail", newsock);
        return false;
    }
    sendstr("OK", newsock);
    if (file.is_open())
    {
        std::string st;
        int i = 0;
        while (getline(file, st))
        {
            if (i == linenumber)
                temp << content;
            temp << st << "\n";
            i++;
        }
        if (linenumber == N)
            temp << content;
    }
    file.close();
    temp.close();
    remove(filename);
    rename("temp", filename);
    sendFile(newsock, filename);
    curr_file->lines = NLine(filename);
    writeFile();
    return true;
}
// remove an invitation
void removeInvite(int cid)
{
    for (int i = 0; i < invites.length(); i++)
    {
        if (invites[i].to_invite == cid)
        {
            invites.remove(i);
            break;
        }
    }
}
// check an invitation
Invite *checkInvite(int cid)
{
    for (int i = 0; i < invites.length(); i++)
    {
        if (invites[i].to_invite == cid)
        {
            return &invites[i];
        }
    }
    return NULL;
}

// Funtion to parse query and handle them
void handleQuery(char *query, int newsock)
{
    // std::cout << "At handle qw \n";
    int line = 5555;
    char s[20] = {0}, lne[1024] = {0};
    char a[20] = {0}, b[20] = {0}, c[20] = {0}, d[20] = {0}, e[20] = {0};

    sscanf(query, "/%[^ \n] %[^ \t\n] %[^ \t\n] %[^ \t\n] %[^ \t\n]", s, b, c, d, e);
    std::string cmd = s;
    if (cmd == "users")
    {
        std::string st;
        for (int i = 0; i < NClients; i++)
        {
            if (users[i] != 0)
            {
                std::string uniqueid = std::to_string(users[i] + 10000);
                st += (uniqueid + '\n');
            }
        }
        sendstr(st.c_str(), newsock);
    }
    else if (cmd == "upload")
    {
        char name[256];
        recievestr(name, newsock, 256);
        for (int i = 0; i < dir.length(); i++)
        {
            if (dir[i].filename == name)
            {
                sendstr("The file is already uploaded at server try different filename\n", newsock);
                return;
            }
        }
        sendstr("OK", newsock);
        recieveFile(name, newsock);
        int lines = NLine(name);
        dir.pushback(File(newsock, b, lines));
        writeFile();
    }
    else if (cmd == "download")
    {
        bool toSend = false;
        toSend = checkPermission(b, newsock) != 0;
        if (toSend)
        {
            sendstr("OK", newsock);
            sendFile(newsock, b);
        }
        else
        {
            sendstr("Fail", newsock);
        }
    }
    else if (cmd == "files")
    {
        std::ofstream ott{"temp", std::ios::out};
        sendstr("OK", newsock);
        for (int i = 0; i < dir.length(); i++)
        {
            ott << dir[i].tostring() << "\n";
        }
        if (dir.length() == 0)
        {
            ott << "No files are at server!\n";
        }
        ott.close();
        sendFile(newsock, "temp");
    }
    else if (cmd == "read")
    {
        bool toSend = false;
        toSend = checkPermission(b, newsock) != 0;
        if (toSend)
        {
            int N = NLine(b);

            if (strlen(c) <= 0)
            {
                readline(b, 0, N - 1, N, newsock);
            }
            else if (strlen(d) <= 0)
            {
                int linenumber = atoi(c);
                readline(b, linenumber, linenumber, N, newsock);
            }
            else
            {
                int linenumber = atoi(c);
                int end = atoi(d);
                readline(b, linenumber, end, N, newsock);
            }
        }
        else
        {
            sendstr("Fail You dont have permission", newsock);
        }
    }
    else if (cmd == "delete")
    {
        bool toSend = false;
        toSend = checkPermission(b, newsock) == 'E';
        if (toSend)
        {
            int N = curr_file->lines;

            if (strlen(c) <= 0)
            {
                del(b, 0, N - 1, N, newsock);
            }
            else if (strlen(d) <= 0)
            {
                int linenumber = atoi(c);
                bool ret = del(b, linenumber, linenumber, N, newsock);
                if (!ret)
                    return;
            }
            else
            {
                int linenumber = atoi(c);
                int end = atoi(d);
                bool ret = del(b, linenumber, end, N, newsock);
                if (!ret)
                    return;
            }
            curr_file->lines = NLine(b);
            writeFile();
        }
        else
        {
            sendstr("Fail You dont have permission", newsock);
        }
    }
    else if (cmd == "insert")
    {
        bool toSend = false;
        toSend = checkPermission(b, newsock) == 'E';
        if (toSend)
        {
            bzero(lne, 1024);
            int N = curr_file->lines;
            int ct = 0;
            while (query[ct] != '"')
                ct++;
            std::string content = &query[ct + 1];
            int a = content.find('"');
            content.replace(a, 1, "");
            content = std::regex_replace(content, std::regex(R"(\\n)"), "\n");
            content = std::regex_replace(content, std::regex(R"(\\t)"), "\t");
            if (c[0] == '"')
            {
                bool ret = insert(b, N, content, N, newsock);
                if (!ret)
                    return;
            }
            else if ((c[0] >= '0' && c[0] < '9') || c[0] == '-')
            {
                int linenumber = atoi(c);
                bool ret = insert(b, linenumber, content, N, newsock);
                if (!ret)
                    return;
            }
            else
            {
                sendstr("Fail Wrong format", newsock);
            }
        }
        else
        {
            sendstr("Fail You dont have permission", newsock);
        }
    }
    else if (cmd == "invite")
    {
        bool file_exist = false, client_exist = false;
        File *f;
        for (int i = 0; i < dir.length(); i++)
        {
            if (dir[i].filename == b)
            {
                file_exist = true;
                f = &dir[i];
                break;
            }
        }
        if (!file_exist)
        {
            sendstr("Fail file does not exist at server ,Please Upload!\n", newsock);
            return;
        }
        if (f->m_owner != newsock)
        {
            sendstr("Fail you are not the owner of the file\n", newsock);
            return;
        }
        int cid = atoi(c);
        cid -= 10000;
        for (int i = 0; i < NClients; i++)
        {
            if (users[i] == cid)
            {
                client_exist = true;
                break;
            }
        }
        if (!client_exist)
        {
            sendstr("Fail Client does not exist!\n", newsock);
            return;
        }
        if (cid == newsock)
        {
            sendstr("Fail You cannot invite youself!\n", newsock);
            return;
        }
        Invite i = {f, cid, d[0], newsock};
        Invite *is_already_invited = checkInvite(i.to_invite);
        invites.pushback(i);
        sendstr("OK invite is sent", newsock);
        std::ostringstream req;
        if (i.P == 'V')
        {
            req << "You have an invite for the file " << f->filename << " as viewer. Do you want to accept it? Answer in Yes or No!\n";
        }
        else
        {
            req << "You have an invite for the file " << f->filename << " as Editor. Do you want to accept it? Answer in Yes or No!\n";
        }
        if(is_already_invited == NULL) sendstr(req.str().c_str(), i.to_invite);
    }
    else if (query[0] == 'Y' || query[0] == 'y')
    {

        Invite *i = checkInvite(newsock);
        if (i != NULL)
        {
            bool client_exist;
            for (int j = 0; j < NClients; j++)
            {
                if (users[j] == i->from)
                {
                    client_exist = true;
                    break;
                }
            }
            if (!client_exist)
            {
                sendstr("Fail The client has exited.", newsock);
                return;
            }
            File *f = i->file;
            f->setColab(newsock, i->P);
            removeInvite(newsock);
            sendstr("Access Granted", newsock);
            sendstr("User has accepted your invite", i->from);
            writeFile();
            Invite *i1 = checkInvite(newsock);
            if (i1 != NULL)
            {
                std::ostringstream req;
                if (i1->P == 'V')
                {
                    req << "You have an invite for the file " << f->filename << " as viewer. Do you want to accept it? Answer in Yes or No!\n";
                }
                else
                {
                    req << "You have an invite for the file " << f->filename << " as Editor. Do you want to accept it? Answer in Yes or No!\n";
                }
                sendstr(req.str().c_str(), i1->to_invite);
            }
        }
        else
        {
            sendstr("Just a normal query", newsock);
            sendstr("Fail You did not have any pending invite", newsock);
        }
    }
    else if (query[0] == 'N' || query[0] == 'n')
    {
        Invite *i = checkInvite(newsock);
        if (i != NULL)
        {
            bool client_exist;
            for (int j = 0; j < NClients; j++)
            {
                if (users[j] == i->from)
                {
                    client_exist = true;
                    break;
                }
            }
            if (!client_exist)
            {
                sendstr("Fail The client has exited.", newsock);
                return;
            }
            File *f = i->file;
            f->removeColab(newsock);
            removeInvite(newsock);
            sendstr("User has rejected your query", i->from);
            writeFile();
            sendstr("Rejected", newsock);
        }
        else
        {
            sendstr("Just a normal query", newsock);
            sendstr("Fail You did not have any pending invite", newsock);
        }
    }
}

using namespace std;
int main()
{
    bzero(users, sizeof(users));
    int server_fd, new_socket, valread, clientSocket[NClients], sd, serverSd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    fd_set readFd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) // create server socket
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

    if (bind(server_fd, (struct sockaddr *)&address, // bind the server
             sizeof(address)) < 0)
    {
        handleError("bind failed");
    }
    if (listen(server_fd, NClients) < 0) // listen to client
    {
        handleError("listen");
    }
    printf("\033[0;32mServer started Waiting for client\n\033[0m");
    printf("Type \033[0;31mCtrl/Cmd+C \033[0m to quit\n");
    int isInvalid = 0;
    bzero(clientSocket, NClients * sizeof(int));
    while (true)
    {
        FD_ZERO(&readFd);
        FD_SET(server_fd, &readFd);
        serverSd = server_fd;

        for (int i = 0; i < NClients; i++)
        {
            sd = clientSocket[i];
            if (sd > 0)
                FD_SET(sd, &readFd);
            if (sd > serverSd)
                serverSd = sd;
        }
        if ((select(serverSd + 1, &readFd, NULL, NULL, NULL) < 0) && (errno != EINTR)) // select a file descriptor
            std::cout << "Select error\n";

        if (FD_ISSET(server_fd, &readFd))
        {
            // accept client request
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, // accept the connection
                                     (socklen_t *)&addrlen)) < 0)
            {
                handleError("Connection exceeded\n");
            }

            cout << "Connected with client socket number " << new_socket << "\n";
            int i;
            for (i = 0; i < NClients; i++)
            {
                if (clientSocket[i] == 0)
                {
                    clientSocket[i] = new_socket; // save the fd into array
                    users[i] = new_socket;

                    current_client++;
                    break;
                }
            }
            if (i == NClients) // if we could not handle more clients
            {

                strcpy(buffer, "Server cannont handle any more connections \n");
                send(new_socket, buffer, 1024, 0);
                printf("%s", buffer);
                close(new_socket);
            }
            else
            {

                std::string sts = std::string("Welcome user You are assigned a client id of ") + std::to_string(new_socket + 10000);
                sendstr(sts.c_str(), new_socket);
            }
        }
        for (int i = 0; i < NClients; i++)
        {
            sd = clientSocket[i];
            // check on which socket request is there
            if (FD_ISSET(sd, &readFd))
            {
                bzero(buffer, 1024);
                valread = recv(sd, buffer, 1024, 0); // waiting to recieve messages
                if (valread < 0)
                {
                    handleError("error on reading from client\n");
                }
                printf("\033[0;33mRequested query is\033[0m %s\n", buffer); // read the message

                if (strncmp(buffer, "/exit", 5) == 0)
                {
                    sendstr("Just a normal query", sd);
                    close(sd);
                    current_client--;
                    clientSocket[i] = 0;
                    for (int j = 0; j < NClients; j++)
                        if (users[j] != 0 && sd == users[j])
                        {
                            for (int k = 0; k < dir.length(); k++)
                            {
                                if (dir[k].m_owner == users[j])
                                {
                                    remove(dir[k].filename.c_str());
                                    dir.remove(k);
                                }
                            }
                            users[j] = 0;
                        }
                    writeFile();
                    continue;
                }
                if (buffer[0] != 'Y' && buffer[0] != 'N' && buffer[0] != 'y' && buffer[0] != 'n')
                    sendstr("Just a normal query", sd);

                handleQuery(buffer, sd); // handle message

                bzero(buffer, 1024);
            }
        }
    }
    return 0;
}