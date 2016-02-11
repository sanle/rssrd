# README #

rssrd - RSS Reader Daemon

### Short Description ###
Daemon fetches remote feeds to create their local mirrors in file system. Daemon supports RSS and Atom. 
 
### Dependencies ###
* Curl
* Mini-XML (2.9)

### Desciprion ###

 Daemon reads remote feeds and appends new data to local file..
Links to feed are read from config file (the file location ~/.config/rssrd/sources).
New data is written into file named `domainname.rss`. Path to the file is read from config file (the file location ~/.config/rssrd/destination).
Daemon fetches feed every 10 minutes.

If -n parameter is passed, the program does not become daemon.

### Config Files ###

*~/.config/rssrd/sources*

Contains list of feed URLs. One link per line.
If the first symbol of the line is # it meant that line is comment.

*~/.config/rssrd/destination*

Contains full path to derictory where deamon will save data. Also contains *.error.log* 

### Contacts ###

* Developed by Alexandr Lebedinskiy aka SanLe
* [sanle@openmailbox.org](mailto:sanle@openmailbox.org)
