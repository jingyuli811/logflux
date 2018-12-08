# logflux
An udp log system monitor

# Prerequesite
* TinyXML2 https://github.com/leethomason/tinyxml2
* WTL10 https://sourceforge.net/projects/wtl/files/WTL%2010/

Download the above libraries and place them in the following arrangemtn.

project(any name you like)
  |-logflux
  |-WTL
  |-tinyxml2

# Build
This is a long time ago project and was started only to support Windows platform. 
You can just use the build_win32.bat to generate visual stuido solution files.
The generated solution can be found in build32 folder.

Warning: Win64 version crashs due to unknown reason.

# How to use
## put together logflux.exe and logflux.xml
These two files needs to be placed in the same folder to work.
logflux.xml is the configuration about log server.

## logflux.xml example 
```
<?xml version="1.0" standalone="no" ?>
<Tabs>
  <Tab name="default">
    <ip>127.0.0.1</ip>
    <port>666</port>
    <bufsize>512</bufsize>
    <loglevels>
      <level type="debug" value="1"/>
      <level type="info" value="1"/>
      <level type="warn" value="1"/>
      <level type="error" value="1"/>
    </loglevels>
    <filters>
      <black>
      </black>
      <white>
      </white>
    </filters>
  </Tab>
</Tabs>
```
This will open a page listing to 127.0.0.1:666 and receive all logs sent to this address.
Adding more tabs is possible, just need to add another <Tab></Tab> section under <Tabs> tag.
  
# Snapshot
![logflux snapshot](/doc/logflux.png)

# TODO
This project may exist many bugs because it had been out of maintenance since years ago.  
