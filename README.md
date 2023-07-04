# Webserver
A low-level implementation of an HTTP server which can handle GET requests. After responding to a request server maintains the connection with an inactive client for 1000ms unless ```Connection : closed``` is included in the request. The server checks the type of the chosen file and sets accordingly the ```Content-Type``` field.

## Possible server status codes
* 200 OK - success
* 301 Moved Permanently - redirect
* 403 Forbidden - access attempt of a file outside of the given directory
* 404 Not found - access attempt of a non-existing file
* 501 Not implemented - wrong data sent to server

## Setup
Add to the file ```/etc/hosts``` the following line:
```
127.0.1.1 virtual-domain.example.com
```
Compile the programm using the command:
```
make
```
## Running the server
After compilation run the server by the command:
```
./webserver directory port
```
where directory stands for a directory from which the server will serve the webpages whearas port is a port number. 

As client use e.g. an Internet browser to send reqeusts
```
http://domain_name:port/page.html
```
Entering the above URL will result in displaying the content of a file ```directory/domain_name/page.html``` where directory is the directory name given as an argument on running the server.
An sample request:
```
http://domain_name:1234/index.html
```
A result of executing the above reqeust would be the same as  of the following one because the server tries always to open index.html file when no file is explicity given.
```
http://domain_name:1234
```