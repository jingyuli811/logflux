#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>
#include "tinyxml2/tinyxml2.h"
#include "loglistview.h"
#include "TabData.h"

class CConfig
{
public:
  CConfig();
  ~CConfig();

  bool LoadXml();
  bool LoadIni();
  bool SaveXml();
  bool SaveIni();
  const CTabInfoArray& TabInfoArray() { return m_arrTabInfo; }

  // file tab
  CTabInfo* NewTabInfo(tstring filepath);

  TFilterPack* GetNullFilterPack() { return &m_nullFilterPack; }

protected:
  bool ParseTab(tinyxml2::XMLNode* tabNode);

protected:
  CTabInfoArray m_arrTabInfo;
  TFilterPack m_nullFilterPack;
};

CConfig& theConfig();


#endif