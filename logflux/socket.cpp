#include "socket.h"

bool SocketInitialize()
{        
  WSADATA wsaData;

  if( WSAStartup(MAKEWORD(1, 1), &wsaData) != 0 )
  {
    // fixme
    //SetError();
    return false;
  }
  return true;
}

void SocketFinalize()
{
  WSACleanup();
}

const char* SocketError()
{
  static char buffer[1024];
  memset(buffer, 0, _countof(buffer));

  ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    WSAGetLastError(),
    0,
    (LPSTR)&buffer,
    _countof(buffer),
    NULL);

  return buffer;
}

Socket::Socket() : m_nError(NOERROR)
{
 
}

Socket::~Socket()
{

}

bool Socket::Create()
{
  // create socket
  if( INVALID_SOCKET == (m_nSockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) )
  {
    SetError();
    return false;
  }
  return true;
}

const TCHAR* Socket::ErrorStr()
{
  static TCHAR buffer[1024];
  memset(buffer, 0, _countof(buffer));

  ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   m_nError,
                   0,
                   (LPTSTR)&buffer,
                   _countof(buffer),
                   NULL);

  return buffer;
}


const char* Socket::ErrorStrA()
{
  static char buffer[1024];
  memset(buffer, 0, _countof(buffer));

  ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    m_nError,
    0,
    (LPSTR)&buffer,
    _countof(buffer),
    NULL);

  return buffer;
}