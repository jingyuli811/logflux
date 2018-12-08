#ifndef VERSION_H
#define VERSION_H 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// ---------------------------------------------------------------------------
// Several defines have to be given before including this file. These are:
// ---------------------------------------------------------------------------
#define TEXT_AUTHOR            vizshala@gmail.com // author (optional value)
#define PRD_MAJVER             1 // major product version
#define PRD_MINVER             0 // minor product version
#define PRD_BUILD              0 // build number for product
#define FILE_MAJVER            1 // major file version
#define FILE_MINVER            0 // minor file version
#define FILE_BUILD             0 // file build number
#define DRV_YEAR               2018 // current year or timespan (e.g. 2003-2009)
#define TEXT_PRODUCTNAME       win log view // product's name
#define TEXT_FILEDESC          win log view // component description
#define TEXT_MODULE            logflux // module name
// #define TEXT_SPECIALBUILD      // optional comment for special builds
#define TEXT_INTERNALNAME      logflux.exe // copyright information
// #define TEXT_COMMENTS          // optional comments

#endif 
