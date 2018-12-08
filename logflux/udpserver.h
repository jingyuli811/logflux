#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "socket.h"

class UdpServer : public Socket
{
public:
  UdpServer();
  virtual ~UdpServer();

  bool Bind(const char* p_sAddr, short p_nPort);
  void Receive();
  virtual void OnReceive(char* p_pData, int p_nLen) {}
  virtual void OnReceiveTerm(){  }

  void Start() { m_bRunning = true; }
  void Stop() { m_bRunning = false; shutdown(m_nSockfd, SD_BOTH); closesocket(m_nSockfd); }
  inline bool IsRunning() { return m_bRunning; }
  inline bool IsTermed() { return m_nSockfd == INVALID_SOCKET; }

private:
  bool m_bRunning;
  bool m_bPause;
};


#endif