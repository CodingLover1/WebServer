# WebServer
>This is a Web Server.It just for learing Network Programming.

### How to use it
* make a dir in /var.like this /var/www/html
* then make a file like index.html
* compile the server:g++ main.cpp http_conn.cpp pool_cgi.cpp -o WebServer -lpthread
* in the browser input http://ip:port/index.html


