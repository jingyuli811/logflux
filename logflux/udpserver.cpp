#include "udpserver.h"

UdpServer::UdpServer() : m_bRunning(false)
{

}

UdpServer::~UdpServer()
{

}

bool UdpServer::Bind(const char* p_sAddr, short p_nPort)
{
  // create IPv4
  memset(&m_sockaddr, 0, sizeof(m_sockaddr));
  m_sockaddr.sin_family = AF_INET;
  m_sockaddr.sin_port = htons(p_nPort);
  m_sockaddr.sin_addr.s_addr = inet_addr(p_sAddr);

  // bind socket
  if( SOCKET_ERROR == bind(m_nSockfd, (struct sockaddr *)&m_sockaddr, sizeof(m_sockaddr)) )
  {
    SetError();
    return false;
  }

  return true;
}

void UdpServer::Receive()
{
  // receive
  const int max_length = 8192;
  char buffer[max_length] = {0};
  struct sockaddr_in from_addr;
  memset(&from_addr, 0, sizeof(from_addr));
  int from_addr_len = sizeof(sockaddr_in);
  int sz;
  while(m_bRunning) 
  {
    sz = recv(m_nSockfd, buffer, _countof(buffer)-8, 0);//(struct sockaddr *)&from_addr, &from_addr_len);
    if (sz <= 0 )
    {      
      SetError();
      break;
    }

    OnReceive(buffer, sz);
  }

  // double checking
  Stop();
  
  m_nSockfd = INVALID_SOCKET;

  OnReceiveTerm();
}
