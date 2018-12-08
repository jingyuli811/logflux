#include "logflux.h"
#include "config.h"
#include "utils/inifile.h"
#include "utils/stringconv.hpp"
#include "utils/systemutils.h"
#include "tinyxml2/tinyxml2.h"
#ifdef TIXML_USE_STL
#include <iostream>
#include <sstream>
using namespace std;
#else
#include <stdio.h>
#endif
#include "TinyXmlHelper.hpp"

CConfig& theConfig()
{
    static CConfig config;
    return config;
}

template< class stringtype >
void StringToken(const stringtype& src,
    const stringtype& delimiters,
    std::vector<stringtype>& tokens)
{
    // skip delimiters at beginning.
    stringtype::size_type lastPos = src.find_first_not_of(delimiters, 0);

    // find first "non-delimiter".
    stringtype::size_type pos = src.find_first_of(delimiters, lastPos);

    while (stringtype::npos != pos || stringtype::npos != lastPos)
    {
        // found a token, add it to the vector.
        tokens.push_back(src.substr(lastPos, pos - lastPos));

        // skip delimiters.  Note the "not_of"
        lastPos = src.find_first_not_of(delimiters, pos);

        // find next "non-delimiter"
        pos = src.find_first_of(delimiters, lastPos);
    }
};

// ------------------------------------------------------------------
CConfig::CConfig()
{
}

// ------------------------------------------------------------------
CConfig::~CConfig()
{
    for (size_t i = 0; i<m_arrTabInfo.size(); i++)
    {
        delete m_arrTabInfo[i];
    }
    m_arrTabInfo.clear();
}

// ------------------------------------------------------------------
bool CConfig::ParseTab(XMLNode* tabNode)
{
    if (!tabNode)
        return false;

    CTabInfo* tabinfo = new CTabInfo();
    if (!tabinfo)
        return false;

    // name
#ifdef _UNICODE
    FindAttrAsWStr(tabNode, tabinfo->m_sTitle, "name");
#else
    FindAttrAsStr(tabNode, tabinfo->m_sTitle, "name");
#endif
    // ip
    FindChildValueAsStr(tabNode, tabinfo->m_sIP, "ip");
    // port
    FindChildValueAsInt(tabNode, tabinfo->m_nPort, "port");
    // bufsize
    FindChildValueAsInt(tabNode, tabinfo->m_nBufSize, "bufsize");

    // loglevel
    XMLElement* levelsElement = tabNode->FirstChildElement("loglevels");
    if (levelsElement)
    {
        XMLElement* levelElement = levelsElement->FirstChildElement("level");
        while (levelElement)
        {

            const char* type = levelElement->Attribute("type");
            const char* value = levelElement->Attribute("value");
            if (type)
            {
                bool enable = (value && atoi(value)>0) || true;
                if (!strcmp(type, "debug"))
                    tabinfo->m_arrbLogLv[LOG_DEBUG] = enable;
                else if (!strcmp(type, "info"))
                    tabinfo->m_arrbLogLv[LOG_INFO] = enable;
                else if (!strcmp(type, "warn"))
                    tabinfo->m_arrbLogLv[LOG_WARN] = enable;
                else if (!strcmp(type, "error"))
                    tabinfo->m_arrbLogLv[LOG_ERROR] = enable;
            }

            levelElement = levelElement->NextSiblingElement();
        }
    }

    // filters
    XMLElement* filtersElement = tabNode->FirstChildElement("filters");
    if (filtersElement)
    {
        // black
        {
            XMLElement* bfilterElement = filtersElement->FirstChildElement("black");
            if (bfilterElement)
            {
                XMLElement* itemElement = bfilterElement->FirstChildElement("item");
                while (itemElement)
                {
                    const char* value = itemElement->Attribute("value");
                    const char* enable = itemElement->Attribute("enable");
                    if (value && value[0])
                    {
                        bool benable = (enable && atoi(enable)>0) || true;
                        tabinfo->m_Filter.m_BlackList.Add(value, strlen(value), benable);
                    }
                    itemElement = itemElement->NextSiblingElement();
                }
            }
        }

        // white
        {
            XMLElement* wfilterElement = filtersElement->FirstChildElement("white");
            if (wfilterElement)
            {
                XMLElement* itemElement = wfilterElement->FirstChildElement("item");
                while (itemElement)
                {
                    const char* value = itemElement->Attribute("value");
                    const char* enable = itemElement->Attribute("enable");
                    if (value && value[0])
                    {
                        bool benable = (enable && atoi(enable)>0) || true;
                        tabinfo->m_Filter.m_WhiteList.Add(value, strlen(value), benable);
                    }
                    itemElement = itemElement->NextSiblingElement();
                }
            }
        }
    }

    m_arrTabInfo.push_back(tabinfo);

    return true;

}
/*
// ------------------------------------------------------------------
bool CConfig::ParseTab(XMLHandle tabHandle)
{
if( !tabHandle.Node() )
return false;

CTabInfo* tabinfo = new CTabInfo();
if( !tabinfo )
return false;

// ip
FindChildValueAsStr(tabHandle, tabinfo->m_sIP, "ip");
FindChildValueAsInt(tabHandle, tabinfo->m_nPort, "port");
FindChildValueAsInt(tabHandle, tabinfo->m_nBufSize, "bufsize");

// loglevel
XMLHandle levelsHandle = tabHandle.FirstChildElement( "loglevels" );
if( levelsHandle.Node() )
{
XMLHandle levelHandle = levelsHandle.FirstChildElement( "level" );
XMLNode* levelNode = levelHandle.Node();
while( levelNode )
{
if( levelNode->Type() == XMLNode::TINYXML_ELEMENT )
{
const char* type = levelNode->ToElement()->Attribute("type");
const char* value = levelNode->ToElement()->Attribute("value");
if( type && value )
{
if( !strcmp(type, "debug") )
tabinfo->m_arrbLogLv[LOG_DEBUG] = atoi( value );
else if( !strcmp(type, "info") )
tabinfo->m_arrbLogLv[LOG_INFO] = atoi( value );
else if( !strcmp(type, "warn") )
tabinfo->m_arrbLogLv[LOG_WARN] = atoi( value );
else if( !strcmp(type, "error") )
tabinfo->m_arrbLogLv[LOG_ERROR] = atoi( value );
}
}
levelNode = levelNode->NextSibling();
}
}

// filters
}
*/

// ------------------------------------------------------------------
bool CConfig::LoadXml()
{
    tinyxml2::XMLDocument doc;
    if (XML_SUCCESS != doc.LoadFile(".\\logflux.xml"))
        return false;

    XMLHandle docHandle(&doc);
    if (!docHandle.ToNode())
        return false;

    // --------------------------------------------------------
    // An example of changing existing attributes, and removing
    // an element from the document.
    // --------------------------------------------------------

    // Tabs
    XMLHandle tabsHandle = docHandle.FirstChildElement("Tabs");
    if (tabsHandle.ToNode())
    {
        XMLHandle tabHandle = tabsHandle.FirstChildElement("Tab");
        XMLNode* tabNode = tabHandle.ToNode();
        while (tabNode)
        {
            ParseTab(tabNode);
            tabNode = tabNode->NextSiblingElement();
        }
    }

    return true;
}

// ------------------------------------------------------------------
bool CConfig::SaveXml()
{
    return true;
}

// ------------------------------------------------------------------
bool CConfig::SaveIni()
{
    return true;
}

// ------------------------------------------------------------------
CTabInfo* CConfig::NewTabInfo(tstring filepath)
{
    CTabInfo* pTabInfo = new CTabInfo();
    if (pTabInfo)
    {
        pTabInfo->m_sFile = filepath;
        pTabInfo->m_sTitle = PathFindFileName(pTabInfo->m_sFile.c_str());
    }

    return pTabInfo;
}