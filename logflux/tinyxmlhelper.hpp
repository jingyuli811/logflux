/** @file TinyXmlHelper.hpp
    @brief helper functions for tinyxml

*/

#ifndef TINY_XML_HELPER_HPP
#define TINY_XML_HELPER_HPP

#include "utils/stringconv.hpp"
#include "utils/stringutils.hpp"
#include "tinyxml2/tinyxml2.h"

using namespace tinyxml2;

/** ----------------------------------------------------------------------------
* get text part of node as string value
* @param element node to be find
* @param name string variable used to store result
*/
inline void FindValueAsStr(XMLNode* node,
                           std::string& name)
{
  if( node )
  {
      XMLNode* thisnode = node->FirstChild();
    if( thisnode )
    {
      name = thisnode->Value();
    } 
  }
}

/** ----------------------------------------------------------------------------
 * get text part of node as string value
 * @param element node to be find
 * @param name string variable used to store result
 */
inline void FindValueAsStr(XMLHandle& element, 
                           std::string& name)
{
  if( element.FirstChild().ToText() ) 
    name = element.FirstChild().ToText()->Value();
}

/** ----------------------------------------------------------------------------
* get text part of node as integer value
* @param element node to be find
* @param name string variable used to store result
*/
inline void FindValueAsInt(XMLHandle& element,
                           std::string& name)
{
  if( element.FirstChild().ToText() ) 
    name = atoi(element.FirstChild().ToText()->Value());
}

/** ----------------------------------------------------------------------------
* get text part of node's specific child as string value
* @param element node to be find
* @param name string variable used to store result
* @param tofind child name 
*/
inline void FindChildValueAsStr(XMLNode* node,
                                std::string& name, 
                                const char* tofind)
{
  if( node )
  {
    XMLElement* element = node->FirstChildElement(tofind);
    if( element )
    {
        XMLNode* cnode = element->FirstChild();
      if( cnode && cnode->Value() )
      {
        name = cnode->Value();
      }
    } 
  }
}

/** ----------------------------------------------------------------------------
* get text part of node's specific child as string value
* @param element node to be find
* @param name string variable used to store result
* @param tofind child name 
*/
inline void FindChildValueAsStr(XMLHandle& element,
                                std::string& name, 
                                const char* tofind)
{
  if(XMLText* text = element.FirstChildElement(tofind).FirstChild().ToText() )
    name = text->Value();
}

/** ----------------------------------------------------------------------------
* get text part of node's specific child as string value
* @param element node to be find
* @param name string variable used to store result
* @param tofind child name 
*/
inline int FindChildValueAsStr(XMLHandle& element,
                                char* name, 
                                int len,
                                const char* tofind)
{
  if(XMLText* text = element.FirstChildElement(tofind).FirstChild().ToText() )
  {
    return StringCopy(name, len, text->Value());
  }
  return FALSE;
}

/** ----------------------------------------------------------------------------
* get text part of node's specific child as wide string value
* @param element node to be find
* @param name string variable used to store result
* @param tofind child name 
*/
inline void FindChildValueAsWStr(XMLHandle& element,
                                 std::wstring& name, 
                                 const char* tofind)
{
  if(XMLText* text = element.FirstChildElement(tofind).FirstChild().ToText() )
  {
    name = CStringConv::AToW(text->Value(), CP_UTF8);
  }
}

/** ----------------------------------------------------------------------------
* get text part of node's specific child as wide string value
* @param element node to be find
* @param name string variable used to store result
* @param tofind child name 
*/
inline int FindChildValueAsWStr(XMLHandle& element,
                                 wchar_t* name, 
                                 int len,
                                 const char* tofind)
{
  if(XMLText* text = element.FirstChildElement(tofind).FirstChild().ToText())
  {
    const char* c = text->Value();
    return StringCopy(name, len, CStringConv::AToW(text->Value(), CP_UTF8).c_str());
  }
  return FALSE;
}


/** ----------------------------------------------------------------------------
* get text part of node's specific child as string value
* @param element node to be find
* @param name string variable used to store result
* @param tofind child name 
*/
inline void FindChildValueAsInt(XMLNode* node,
                                int& name, 
                                const char* tofind)
{
  if( node )
  {
    XMLElement* element = node->FirstChildElement(tofind);
    if( element )
    {
        XMLNode* cnode = element->FirstChild();
      if( cnode && cnode->Value() )
      {
        name = atoi( cnode->Value() );
      }
    } 
  }
}

/** ----------------------------------------------------------------------------
* get text part of node's specific child as integer value
* @param element node to be find
* @param name string variable used to store result
* @param tofind child name 
*/
inline void FindChildValueAsInt(XMLHandle& element,
                                int& name, 
                                const char* tofind)
{
  if(XMLText* text = element.FirstChildElement(tofind).FirstChild().ToText())
    name = atoi(text->Value());
}

/** ----------------------------------------------------------------------------
* get attribute of node's specific child as string value
* @param element node to be find
* @param attrval string variable used to store result
* @param tofind child name 
* @param attrname attribute
*/
inline void FindChildAttrAsStr(XMLHandle& element,
                               std::string& attrval, 
                               const char* tofind, 
                               const char* attrname)
{
  if( XMLElement* child = element.FirstChildElement(tofind).ToElement() )
  {
    const char* sval = child->Attribute(attrname);
    attrval = sval ? sval : "";
  }
}


/** ----------------------------------------------------------------------------
* get attribute of node's specific child as string value
* @param element node to be find
* @param attrval string variable used to store result
* @param tofind child name 
* @param attrname attribute
* @return TRUE if found, FALSE otherwise
*/
inline int FindChildAttrAsStr(XMLHandle& element,
                               char* attrval, 
                               int len,
                               const char* tofind, 
                               const char* attrname)
{
  if( XMLElement* child = element.FirstChildElement(tofind).ToElement() )
  {
    const char* sval = child->Attribute(attrname);
    if( sval )
      return StringCopy(attrval, len, sval);
  }
  return FALSE;
}


/** ----------------------------------------------------------------------------
* get attribute of node's specific child as wide string value
* @param element node to be find
* @param attrval string variable used to store result
* @param tofind child name 
* @param attrname attribute
*/
inline void FindChildAttrAsWStr(XMLHandle& element,
                                std::wstring& attrval, 
                                const char* tofind, 
                                const char* attrname)
{
  if(XMLElement* child = element.FirstChildElement(tofind).ToElement())
  {
    const char* sval = child->Attribute(attrname);
    attrval = sval ? CStringConv::AToW(sval, CP_UTF8) : L"";
  }
}

/** ----------------------------------------------------------------------------
* get attribute of node's specific child as wide string value
* @param element node to be find
* @param attrval string variable used to store result
* @param tofind child name 
* @param attrname attribute
* @return TRUE if found, FALSE otherwise
*/
inline int FindChildAttrAsWStr(XMLHandle& element,
                                wchar_t* attrval, 
                                int len,
                                const char* tofind, 
                                const char* attrname)
{
  if(XMLElement* child = element.FirstChildElement(tofind).ToElement())
  {
    const char* sval = child->Attribute(attrname);
    if( sval )
      return StringCopy(attrval, len, CStringConv::AToW(sval, CP_UTF8).c_str());
  }
  return FALSE;
}

/** ----------------------------------------------------------------------------
* get attribute of node's specific child as integer value
* @param element node to be find
* @param attrval string variable used to store result
* @param tofind child name 
* @param attrname attribute
*/
inline void FindChildAttrAsInt(XMLHandle& element,
                               int& attrval, 
                               const char* tofind, 
                               const char* attrname)
{
  if(XMLElement* child = element.FirstChildElement(tofind).ToElement())
  {
    const char* sval = child->Attribute(attrname);
    attrval = sval ? atoi(sval) : 0;
  }
}


/** ----------------------------------------------------------------------------
* get attribute of node as string
* @param element node to be find
* @param attrval string variable used to store result
* @param attrname attribute
*/
inline void FindAttrAsStr(XMLNode* node,
                          std::string& attrval, 
                          const char* attrname)
{
  if( node && node->ToElement() )
  {
    const char* sval = node->ToElement()->Attribute(attrname);
    attrval = sval ? sval : "";
  }
}

/** ----------------------------------------------------------------------------
* get attribute of node as string
* @param element node to be find
* @param attrval string variable used to store result
* @param attrname attribute
*/
inline void FindAttrAsWStr(XMLNode* node,
                          std::wstring& attrval, 
                          const char* attrname)
{
  if( node && node->ToElement() )
  {
    const char* sval = node->ToElement()->Attribute(attrname);
    attrval = sval ? CStringConv::AToW(sval, CP_UTF8) : L"";
  }
}

/** ----------------------------------------------------------------------------
* get attribute of node as string
* @param element node to be find
* @param attrval string variable used to store result
* @param attrname attribute
*/
inline void FindAttrAsStr(XMLHandle& element,
                          std::string& attrval, 
                          const char* attrname)
{
  if( element.ToElement() )
  {
    const char* sval = element.ToElement()->Attribute(attrname);
    attrval = sval ? sval : "";
  }
}

/* ----------------------------------------------------------------------------
* get attribute of node as string
* @param element node to be find
* @param attrval string variable used to store result
* @param attrname attribute
* @return TRUE if found, FALSE otherwise
*/
inline int FindAttrAsStr(XMLHandle& element,
                          char* attrval, 
                          int len,
                          const char* attrname)
{
  if( element.ToElement() )
  {
    const char* sval = element.ToElement()->Attribute(attrname);
    if( sval )
      return StringCopy(attrval, len, sval);
  }
  return FALSE;
}

/** ----------------------------------------------------------------------------
* get attribute of node as wide string
* @param element node to be find
* @param attrval string variable used to store result
* @param attrname attribute
*/
inline void FindAttrAsWStr(XMLHandle& element,
                          std::wstring& attrval, 
                          const char* attrname)
{
  if( element.ToElement() )
  {
    const char* sval = element.ToElement()->Attribute(attrname);
    attrval = sval ? CStringConv::AToW(sval, CP_UTF8) : L"";
  }
}

/** ----------------------------------------------------------------------------
* get attribute of node as wide string
* @param element node to be find
* @param attrval string variable used to store result
* @param attrname attribute
* @return TRUE if found, FALSE otherwise
*/
inline int FindAttrAsWStr(XMLHandle& element,
                           wchar_t* attrval, 
                           int len,
                           const char* attrname)
{
  if( element.ToElement() )
  {
    const char* sval = element.ToElement()->Attribute(attrname);
    if( sval )
      return StringCopy(attrval, len, CStringConv::AToW(sval, CP_UTF8).c_str());
  }
  return FALSE;
}

/** ----------------------------------------------------------------------------
* get attribute of node as integer
* @param element node to be find
* @param attrval string variable used to store result
* @param attrname attribute
*/
inline void FindAttrAsInt(XMLHandle& element,
                          int& attrval, 
                          const char* attrname)
{
  if( element.ToElement() )
  {
    const char* sval = element.ToElement()->Attribute(attrname);
    if( sval )
      attrval = atoi(sval);
  }
}

#endif // TINY_XML_HELPER_HPP