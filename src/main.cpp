#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

void manageClient(SOCKET listening, sockaddr_in client, int clientSize)
{
  SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
  if(clientSocket == INVALID_SOCKET)
  {
    std::cerr << "Failed to connect to client" << std::endl;
    std::terminate();
  }

  char host[NI_MAXHOST];
  char service[NI_MAXSERV];

  ZeroMemory(host, NI_MAXHOST);
  ZeroMemory(service, NI_MAXSERV);

  if(getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
  {
    std::cout << host << " connected on port " << service << std::endl;
  }
  else
  {
    inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
    std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
  }

  // loop accept and echo
  char buf[4096]; // <-- bad!

  while(true)
  {
    ZeroMemory(buf, 4096);

    // wait for client to send something
    int bytesGet = recv(clientSocket, buf, 4096, 0); // waits for message

    if(bytesGet == SOCKET_ERROR)
    {
      std::cerr << "Error on recv()" << std::endl;
      std::terminate();
    }
    if(bytesGet == 0)
    {
      std::cout << "Client disconnected" << std::endl;
      break;
    }

    send(clientSocket, buf, bytesGet + 1, 0);
  }

  // close socket
  closesocket(clientSocket);
}

int main()
{
  // Start winsock
  WSADATA wsData;
  WORD ver = MAKEWORD(2, 2);

  int wsOK = WSAStartup(ver, &wsData);
  if(wsOK != 0)
  {
    std::cerr << "Failed to initialize winsock" << std::endl;
    return -1;
  }
  
  // create socket
  SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
  if(listening == INVALID_SOCKET)
  {
    std::cerr << "Failed to initialize socket" << std::endl;
  }

  // bind to socket
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(54000);
  hint.sin_addr.S_un.S_addr = INADDR_ANY; // or inet_pton

  bind(listening, (sockaddr*)&hint, sizeof(hint));

  // make socket listen
  listen(listening, SOMAXCONN);
  
  // wait for connection
  sockaddr_in client;
  int clientSize = sizeof(client);

  std::thread worker(manageClient, listening, client, clientSize);

  std::cout << "---------------PRESS ENTER TO EXIT SERVER!---------------" << std::endl;
  std::cin.get();
  worker.join();
  // close listening
  closesocket(listening);

  // close winsock
  WSACleanup();
}