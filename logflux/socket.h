#ifndef SOCKET_H
#define SOCKET_H

#include <winsock2.h>
#pragma comment (lib, "ws2_32")

// WSA
bool SocketInitialize();
void SocketFinalize();
const char* SocketError();

class Socket
{
public:
  Socket();
  ~Socket();

  // connection
  bool Create();
  const sockaddr_in& SockAddr() { return m_sockaddr; }
  const short Port() { return m_nPort; }
  const int Sock() { return m_nSockfd; }

  // error
  inline int Error() { return m_nError; }
  const TCHAR* ErrorStr();
  const char* ErrorStrA();

protected:
  inline void SetError() { m_nError = WSAGetLastError(); }

protected:
  int m_nError; // last error

  struct sockaddr_in m_sockaddr;
  short m_nPort;
  int m_nSockfd;
};

#endif