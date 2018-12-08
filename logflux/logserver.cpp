#include "logflux.h"
#include "logserver.h"
#include <process.h>
#include <assert.h>

LogServer::LogServer(ILogServerDelegate* p_pDelegate) :
m_hThread(NULL)
, m_uBufReadIndex(0)
, m_uBufWriteIndex(0)
, m_lBufAvailable(0)
, m_pDelegate(p_pDelegate)
{
  m_sAddr[0] = '\0';
  m_nPort = 0;
}

LogServer::~LogServer()
{
  End();
}

void LogServer::Initialize(const char* p_sAddr, short p_nPort)
{
  // copy addr & port
  SetHost(p_sAddr);
  SetAddrPort(p_nPort);
}

void LogServer::SetHost(const char* p_sAddr)
{
  size_t len = strlen(p_sAddr);
  strncpy(m_sAddr, p_sAddr, len);
  m_sAddr[len] = '\0';
}

void LogServer::SetAddrPort(short p_nPort)
{
  m_nPort = p_nPort;
}

bool LogServer::Run(const char* p_sAddr, short p_nPort)
{
  if( !p_sAddr )
    return false;

  if( IsRunning() )
    return false;

  Start();

  // copy addr & port
  size_t len = strlen(p_sAddr);
  strncpy(m_sAddr, p_sAddr, len);
  m_sAddr[len] = '\0';

  m_nPort = p_nPort;

  // create socket
  if( !Create() )
  {
    Stop();
    return false;
  }

  // bind
  if( !Bind(m_sAddr, m_nPort) )
  {
    Stop();
    return false;
  }

  
  // create thread for receiving
  unsigned tid;
  
  m_hThread = (HANDLE)_beginthreadex( NULL, 
                                      0, 
                                      &LogServer::ReceiveThread, 
                                      (LPVOID)this, 
                                      0, 
                                      &tid );

  return m_hThread != NULL;
}

void LogServer::End()
{
  Dbgp("LogServer::End, thread[0x%08x]\n", m_hThread);
  if( m_hThread )
  {
    Dbgp("LogServer::End, Stop\n");
    Stop();
    Dbgp("LogServer::End, Wait\n");
    WaitForSingleObject(m_hThread, INFINITE);
    Dbgp("LogServer::End, Close\n");
    CloseHandle(m_hThread);
    m_hThread = NULL;
  }
}

void LogServer::Write(const char* data, unsigned int len)
{
  if( m_uBufWriteIndex >= DataBufCount )
    m_uBufWriteIndex = 0;

  if( len >= DataBufSize-1 )
    len = DataBufSize-1;
  memcpy(m_DataBuf[m_uBufWriteIndex], data, len);
  m_DataBuf[m_uBufWriteIndex++][len] = '\0';

  InterlockedIncrement(&m_lBufAvailable);
}

const char* LogServer::Read()
{
  if( m_lBufAvailable )
  {
    InterlockedDecrement(&m_lBufAvailable);

    if( m_uBufReadIndex >= DataBufCount )
      m_uBufReadIndex = 0;
    return m_DataBuf[m_uBufReadIndex++];
  }

  return NULL;
}

unsigned int LogServer::ReceiveThread(void* p_pParam)
{
  LogServer* pServer = reinterpret_cast<LogServer*>(p_pParam);
  assert(pServer);

  pServer->Receive();

  return 0;
}

void LogServer::OnReceive(char* p_pData, int p_nLen)
{
  //Write(p_pData, (unsigned int)p_nLen);
  //PostMessage(m_hNotifyWnd, m_nNotifyMessage, 0, 0);
  if( m_pDelegate )
    m_pDelegate->OnReceive(p_pData, p_nLen);
}

void LogServer::OnReceiveTerm()
{
  //Dbgp("OnReceiveTerm %s:%d\n", m_sAddr, m_nPort);
  if( m_pDelegate )
    m_pDelegate->OnReceiveTerm();

}