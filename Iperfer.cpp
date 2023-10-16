#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <chrono>

using namespace std;

int server(int, char **);
int client(int, char **);

int main(int argc, char *argv[])
{
    cout << "begin" << endl;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0)
        {
            cout << "client mode" << endl;
            client(argc, argv);
        }
        if (strcmp(argv[i], "-s") == 0)
        {
            cout << "server mode" << endl;
            server(argc, argv);
        }
    }

    return 0;
}

int client(int argc, char *argv[])
{
    // std::cout << argc << std::endl;
    if (argc != 8)
    {
        cout << "Error: missing or additional arguments" << endl;
        return -1;
    }

    std::string serverHost = "";
    int port = -1;
    int time = 0;
    int duration = 0;

    for (int i = 0; i < argc; i++)
    {
        //  setting host
        if (strcmp(argv[i], "-h") == 0)
        {
            serverHost = argv[++i];
        }
        //  setting host port
        if (strcmp(argv[i], "-p") == 0)
        {
            port = atoi(argv[++i]);
            if (port < 1024 || 65535 < port)
            {
                cout << "Error: port number must be in the range 1024 to 65535" << endl;
                return -1;
            }
        }
        //  setting duration in seconds for which data should be generated
        if (strcmp(argv[i], "-t") == 0)
        {
            duration = atoi(argv[++i]);
        }
    }

    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        std::cerr << "Error: Unable to create socket." << std::endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, serverHost.c_str(), &serverAddr.sin_addr) <= 0)
    {
        std::cerr << "Error: Invalid server address." << std::endl;
        return 1;
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Error: Connection failed." << std::endl;
        return 1;
    }

    // Send data to the server
    char data[1000];                 // 1 kB
    memset(data, 'A', sizeof(data)); // Fill with data to send

    auto startTime = std::chrono::high_resolution_clock::now();
    while (true)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsedSeconds >= duration)
        {
            break;
        }
        send(clientSocket, data, sizeof(data), 0);
        time++;
    }
    cout << "sent = " << time << " KB, Rate = " << time / duration / 1000 * 8 << " Mbps" << endl;

    close(clientSocket);
}

int server(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << "Error: missing or additional arguments" << endl;
    }

    int port;
    for (int i = 0; i < argc; i++)
    {
        //  setting host port
        if (strcmp(argv[i], "-p") == 0)
        {
            port = atoi(argv[++i]);
            if (port < 1024 || 65535 < port)
            {
                cout << "Error: port number must be in the range 1024 to 65535" << endl;
                return -1;
            }
        }
    }

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error: Unable to create socket." << std::endl;
        return 1;
    }

    // Bind socket to the specified port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Error: Bind failed." << std::endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) == -1)
    {
        std::cerr << "Error: Listen failed." << std::endl;
        return 1;
    }

    std::cout << "Iperfer server listening on port " << port << std::endl;

    // Accept incoming connection
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == -1)
    {
        std::cerr << "Error: Accept failed." << std::endl;
        return 1;
    }

    // Receive data from the client
    char buffer[1000];
    long long totalBytesReceived = 0;
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        totalBytesReceived += bytesRead;
    }

    close(clientSocket);
    close(serverSocket);

    std::cout << "Received " << totalBytesReceived << " bytes." << std::endl;

    return 0;
}