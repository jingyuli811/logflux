#ifndef LOG_SERVER_H
#define LOG_SERVER_H

#include "udpserver.h"

class ILogServerDelegate
{
public:
  explicit ILogServerDelegate(){}
  ~ILogServerDelegate(){}

  virtual void OnReceive(char* p_pData, int p_nLen) = 0;
  virtual void OnReceiveTerm() = 0;
};

class LogServer : public UdpServer
{
public:
  explicit LogServer(ILogServerDelegate* p_pDelegate = NULL);
  ~LogServer();

  void Initialize(const char* p_sAddr, short p_nPort);
  bool Run(const char* p_sAddr, short p_nPort);
  void End();
  void Write(const char* data, unsigned int len);
  const char* Read();
  virtual void OnReceive(char* p_pData, int p_nLen);
  virtual void OnReceiveTerm();

  void SetHost(const char* host);
  void SetAddrPort(short port);

  const char* GetHost() { return m_sAddr; }
  short GetPort() { return m_nPort; }

protected:
  void Clear();

  short m_nPort;
  char m_sAddr[MAX_PATH];

private:
  static unsigned int __stdcall ReceiveThread(void* p_pParam);

private:
  HANDLE m_hThread;
  
  // ring buffer
  static const unsigned int DataBufCount = 128;
  static const unsigned int DataBufSize = 4096;
  unsigned m_uBufWriteIndex;
  unsigned m_uBufReadIndex;
  LONG m_lBufAvailable;
  char m_DataBuf[DataBufCount][DataBufSize];
  ILogServerDelegate* m_pDelegate;
};

#endif