#include "inifile.h"
#include "stringutils.hpp"

// ------------------------------------------------------------------
IniFile::IniFile(const TCHAR* p_sFile) : m_sFile(p_sFile), 
                                                m_nCurSection(0),
                                                m_nCurField(0),
                                                m_bOpen(false)
{
}

// ------------------------------------------------------------------
IniFile::~IniFile()
{
  Clear();
}

// ------------------------------------------------------------------
void IniFile::Clear()
{
  for(TItArrSection itSec = m_arrSection.begin();
    itSec != m_arrSection.end();
    itSec++)
  {
    TIniSection* pSec = *itSec;
    for(TItArrNameVal itNv = pSec->m_arrNameVal.begin();
      itNv != pSec->m_arrNameVal.end();
      itNv++)
    {
      TIniNameVal* pNv = *itNv;
      delete pNv;
    }
    pSec->m_arrNameVal.clear();
    delete pSec;
  }
  m_arrSection.clear();
}

// ------------------------------------------------------------------
void IniFile::Open(const TCHAR* p_sFile)
{
  if( p_sFile && p_sFile[0] )
    m_sFile = p_sFile;

  if( m_sFile.length() <= 0 )
    return;

  Parse();
}

// ------------------------------------------------------------------

void IniFile::SetSection(const char* p_sSec)
{
  if( !p_sSec || !p_sSec[0] )
    return;

  for( size_t i=0; i<m_arrSection.size(); i++ )
  {
    if( !m_arrSection[i]->m_sSection.compare(p_sSec) )
    {
      m_nCurSection = i;
      break;
    }
  }
}

// ------------------------------------------------------------------

void IniFile::SetSection(size_t index)
{
  if( index < m_arrSection.size() )
  {
    m_nCurSection = index;
  }
}

// ------------------------------------------------------------------
void IniFile::Parse()
{
#ifdef _WIN32
  FILE* fp = NULL;
  errno_t err = _tfopen_s(&fp, m_sFile.c_str(), _T("r"));
  if( err )
    return;
#else
  FILE* fp = fopen(m_sFile.c_str(), "r");
  if( !fp )
    return;
#endif

  char line[1024] = {0};
  TIniSection* pCurSec = NULL;
  char* pCtx = NULL;
  while( fgets(line, _countof(line), fp) )
  {
    if( ferror(fp) )
      break;

    if( !line[0] || 
        line[0] == ';' ||
        line[0] == '#' )
      continue;

    if( line[0] == '[' )
    {
#ifdef _WIN32
      char* section = strtok_s(line, " []\r\n", &pCtx);
#else
      char* section = strtok(line, " []\r\n");
#endif
      if(section)
      {
        pCurSec = new TIniSection(section);
        m_arrSection.push_back(pCurSec);
      }
      continue;
    }

#ifdef _WIN32
    char* pName = strtok_s(line, "=", &pCtx);
#else
    char* pName = strtok(line, " =");
#endif
    if( pName )
    {
#ifdef _WIN32
      char* pVal = strtok_s(NULL, "=\r\n", &pCtx);
#else
      char* pVal = strtok(NULL, " =\r\n");
#endif
      if( pVal && pCurSec )
      {
        StringTrim(&pName, ' ');
        StringTrim(&pVal, ' ');
        pCurSec->m_arrNameVal.push_back(new TIniNameVal(pName, pVal));
      }
    }
  }

  // default section
  if( m_arrSection.size() )
    m_nCurSection = 0;

  fclose(fp);
  m_bOpen = true;
}

// ------------------------------------------------------------------
const char* IniFile::Search( std::string p_sName)  
{ 
  if( m_nCurSection >= m_arrSection.size() )
    return NULL;

  TIniSection* pSec = m_arrSection[m_nCurSection];
  for(TItArrNameVal itNv = pSec->m_arrNameVal.begin();
    itNv != pSec->m_arrNameVal.end();
    itNv++)
  {
    TIniNameVal* pNv = *itNv;
    if( !pNv->m_sName.compare(p_sName) )
    {
      return pNv->m_sValue.c_str();
    }
  }

  return NULL;
}

// ------------------------------------------------------------------

const char* IniFile::GetSectionName(size_t index) const
{
  if( index > m_arrSection.size() )
    return "";

  return m_arrSection[index]->m_sSection.c_str();
}

// ------------------------------------------------------------------

size_t IniFile::GetFieldCount(size_t index)
{
  if( index < m_arrSection.size() )
    return m_arrSection[index]->m_arrNameVal.size();

  return 0;
}

// ------------------------------------------------------------------

size_t IniFile::GetFieldCount(const char* p_sSec)
{
  for(size_t i=0; i<m_arrSection.size(); i++)
  {
    if( !m_arrSection[i]->m_sSection.compare(p_sSec) )
      return m_arrSection[i]->m_arrNameVal.size();
  }
  return 0;
}

// ------------------------------------------------------------------
int IniFile::GetFieldIndex(const char* p_sSec)
{
  for(size_t i=0; i<m_arrSection.size(); i++)
  {
    if( !m_arrSection[i]->m_sSection.compare(p_sSec) )
      return i;
  }
  return -1;
}

// ------------------------------------------------------------------
const char* IniFile::GetFieldName(size_t sec, size_t index)
{
  if( sec >= 0 
    && sec < m_arrSection.size() 
    && index >= 0 
    && index < m_arrSection[sec]->m_arrNameVal.size() )
  {
    return m_arrSection[sec]->m_arrNameVal[index]->m_sName.c_str();
  }
  return "";
}

// ------------------------------------------------------------------
const char* IniFile::GetFieldValue(size_t sec, size_t index)
{
  if( sec >= 0 
    && sec < m_arrSection.size() 
    && index >= 0 
    && index < m_arrSection[sec]->m_arrNameVal.size() )
  {
    return m_arrSection[sec]->m_arrNameVal[index]->m_sValue.c_str();
  }
  return "";
}

// ------------------------------------------------------------------

int IniFile::ReadInt(const char* p_sName, const int p_nDefault )
{
  int ret = p_nDefault;

  const char* sF = Search(p_sName);
  if( sF )
    ret = atoi(sF);

  return ret;
}

// ------------------------------------------------------------------
double IniFile::ReadDouble(const char* p_sName, double p_lfDefault )
{
  double ret = p_lfDefault;

  const char* sF = Search(p_sName);
  if( sF )
    ret = atof(sF);

  return ret;
}

// ------------------------------------------------------------------
const char* IniFile::ReadString(const char* p_sName, const char* p_sDefault )
{
  const char* sF = Search(p_sName);
  return (sF ? sF : p_sDefault);
}

// ------------------------------------------------------------------
bool IniFile::ReadBool(const char* p_sName, const bool p_bDefault )
{
  const char* sF = Search(p_sName);
  return (sF ? atoi(sF) > 0 : p_bDefault);
}