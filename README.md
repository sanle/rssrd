# README #

rssrd - RSS Reader Daemon

### Short Description ###
Daemon fetches remote feeds to create their local mirrors in file system. Deamon supports RSS and Atom. 
 
### Dependencies ###
* Curl
* Mini-XML (2.9)

### Desciprion ###

 Daemon reads remote feeds and appends new data to local file..
Links to feed are read from config file (the file location ~/.config/rssrd/sources).
New data is written into file named `domainname.rss`. Path to the file is read from config file (the file location ~/.config/rssrd/destination).
Daemon fetches feed every 10 minutes.  

### Config Files ###

*~/.config/rssrd/sources*

Contains list of feed URLs. One link per line.

*~/.config/rssrd/destination*

Contains full path to derictory where deamon will save data. 

### Contacts ###

* Developed by Alexandr Lebedinskiy aka SanLe
* [sanle@openmailbox.org](mailto:sanle@openmailbox.org)