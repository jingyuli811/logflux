#ifndef VI_INIFILE_H
#define VI_INIFILE_H

#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include <tchar.h>

#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

class IniFile
{
public:
  IniFile(const TCHAR* p_sFile = _T(""));
  ~IniFile();

  void Open(const TCHAR* p_sFile = _T(""));
  void Open(const tstring& p_sFile) { Open(p_sFile.c_str()); }
  bool IsOpen() { return m_bOpen; }

  void SetSection(const char* p_sSec = "");
  void SetSection(size_t index);
  int GetSectionCount() { return m_arrSection.size(); }
  const char* GetSectionName(size_t index ) const;

  size_t GetFieldCount(size_t index);         // by index
  size_t GetFieldCount(const char* p_sSec);   // by section name
  int GetFieldIndex(const char* p_sSec);
  const char* GetFieldName(size_t sec, size_t index);
  const char* GetFieldValue(size_t sec, size_t index);

  // read field data
  int ReadInt(const char* p_sName, const int p_nDefault = 0 );
  double ReadDouble(const char* p_sName, double p_lfDefault = 0.0 );
  const char* ReadString(const char* p_sName, const char* p_sDefault = "");
  bool ReadBool(const char* p_sName, const bool p_bDefault = false);

private:
  void Parse();
  const char* Search( std::string p_sName);
  void Clear();

private:
  struct TIniNameVal
  {
    TIniNameVal(const char* p_sName = "", const char* p_sVal = "") : m_sName(p_sName), m_sValue(p_sVal) {}
    std::string m_sName;
    std::string m_sValue;
  };

  typedef std::vector<TIniNameVal*>  TArrNameVal;
  typedef TArrNameVal::iterator TItArrNameVal;
  struct TIniSection
  {
    TIniSection(const char* p_sSec = "") : m_sSection(p_sSec) {}
    std::string m_sSection;
    TArrNameVal m_arrNameVal;
  };

  typedef std::vector<TIniSection*> TArrSection;
  typedef TArrSection::iterator TItArrSection;

  typedef std::map<std::string, TIniSection*> TMapSection;
  typedef std::pair<std::string, TIniSection*> TPairSection;
  typedef TMapSection::iterator TItMapSection;

  // for sequential iteration
  TArrSection m_arrSection;
  // for non-sequential iteration 
  //TMapSection m_mapSection;

  tstring m_sFile;
  size_t m_nCurSection;
  size_t m_nCurField;
  bool m_bOpen;
};

#endif



