# FastCGI

HHVM has built-in support for two server types: [Proxygen](/hhvm/basic-usage/proxygen) and FastCGI.

FastCGI provides a high performance interface between your codebase and web server (e.g., persistent processes between requests, etc.), but which will also obviously require a front-end compatible web server to serve the requests (e.g., [nginx](http://nginx.org/)).

## How FastCGI Works

HHVM-FastCGI works much the same way as [PHP-FPM](http://php-fpm.org/). HHVM, running in FastCGI mode, is started independently of the web server (Apache, nginx, etc). It listens on either a TCP socket (conventionally localhost:9000) or a UNIX socket. The web server listens on port 80 or port 443 like it normally would. When a new request comes in, the web server either makes a connection to the application server or reuses one of the previously open connections, and communicates using FastCGI protocol. Therefore, the web server continues to decode HTTP protocol, and supplies HHVM with information like the path of the file to be executed, request headers, and body. HHVM computes the response and sends it back to the web server using FastCGI again. Finally, the web server is in charge of sending the HTTP response to the client.

## Using FastCGI

To run the server in FastCGI mode pass the following parameters to hhvm runtime:

    hhvm --mode server -d hhvm.server.type=fastcgi -d hhvm.server.port=9000

The server will now accept connections on localhost:9000. To use a UNIX socket, use the `Server.FileSocket` option instead:

    hhvm --mode server -d hhvm.server.type=fastcgi -d hhvm.server.file_socket=/var/run/hhvm/sock

To turn the server into a daemon, change the value of mode:

    hhvm --mode daemon -d hhvm.server.type=fastcgi -d hhvm.server.file_socket=/var/run/hhvm/sock

Note, all the usual options that are accepted by hhvm runtime can be used in FastCGI mode as well. In particular, `-d hhvm.admin_server.port=9001` will create an additional "admin" server listening on a port 9001.

### Making it work with Apache 2.4

Ensure that you have apache installed via something like:

```
sudo apt-get install apache2
```

The recommended way of integrating with Apache is using `mod_proxy` `mod_proxy_fcgi`. Enable the modules, then in your Apache configuration, add a line as so:

```
ProxyPass / fcgi://127.0.0.1:9000/path/to/your/www/root/goes/here/
# Or if you used a unix socket
# ProxyPass / unix://var/run/hhvm/sock|fcgi://127.0.0.1:9000/path/to/your/www/root/goes/here/
```

This will route *all* the traffic to the FastCGI server. If you want to route only certain requests (e.g. only those from a subdirectory or ending *.php, you can use `ProxyPassMatch`, e.g.

```
ProxyPassMatch ^/(.*\.php(/.*)?)$ fcgi://127.0.0.1:9000/path/to/your/www/root/goes/here/$1
```

Consult `mod_proxy_fcgi` docs for more details on how to use `ProxyPass` and `ProxyPassMatch`.

Also make sure to set up a `DirectoryIndex` in your Apache configuration like this:

```
<Directory /path/to/your/www/root/>
    DirectoryIndex index.php
</Directory>
```

This will try to access `index.php` when you send a request to a directory.

### Making it work with nginx

Ensure that you have nginx installed via something like:

```
sudo apt-get install nginx
```

Now nginx needs to be configured to know where your PHP files are and how to forward them to HHVM to execute. The relevant bit of nginx config lives at `/etc/nginx/sites-available/default` -- by default, it's looking in `/usr/share/nginx/html` for files to serve, but it doesn't know what to do with PHP.

Our included script `sudo /opt/hhvm/<version>/share/hhvm/install_fastcgi.sh` will configure nginx correctly for stock installs. The important part is that it adds `include hhvm.conf` near the top of of the nginx config mentioned above -- this will direct nginx to take any file that ends in `.hh` or `.php` and send it to HHVM via fastcgi.

The default FastCGI configuration from Nginx should work just fine with HHVM-FastCGI. For instance you might want to add the following directives inside one of your `location` directives:

```
root /path/to/your/www/root/goes/here;
fastcgi_pass   127.0.0.1:9000;
# or if you used a unix socket
# fastcgi_pass   unix:/var/run/hhvm/sock;
fastcgi_index  index.php;
fastcgi_param  SCRIPT_FILENAME $document_root$fastcgi_script_name;
include        fastcgi_params;
```

The traffic for the surrounding `location` directive will be now routed to the HHVM-FastCGI server.

*Testing*

To test that everything is working, write this hello world script to `hello.php` in whichever directory nginx looks for its web files -- depending on OS, this may be `/usr/share/nginx/html/hello.php` or `/var/www/html/hello.php` or somewhere else:

```
<?php
echo "Hello world!\n";
```

If the HHVM server is not running, start it up:

```
hhvm -m server -d hhvm.server.type=fastcgi -d hhvm.server.port=9000
```

and then load [http://localhost/hello.php](http://localhost/hello.php) and verify you see "Hello world".

Note that by default `/usr/share/nginx/html` is only writable by root; use `chown` to set permissions as appropriate, or point `/etc/nginx/sites-available/default` at a different root, or [refer to the nginx documentation](http://nginx.org/en/) to do something more fancy. Basically at this point you know things are working, so you can start from a known-good state as you start customizing your configuration, so it's easy to know if things break later which change broke it.

*Admin Server in Nginx*

If you ran with `-vAdminServer.Port=9001` then something like this would work:

```
location ~ {
    fastcgi_pass   127.0.0.1:9001;
    include        fastcgi_params;
}
```
