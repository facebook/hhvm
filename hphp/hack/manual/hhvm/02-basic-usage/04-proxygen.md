# Proxygen

HHVM has built-in support for two server types: Proxygen and [FastCGI](/hhvm/advanced-usage/fastCGI).

Proxygen is a full web server built directly into HHVM, and is recommended since it is generally the easiest to get up and running. It serves web requests *fast*.  Proxygen provides you a high performance web server that is equivalent to what something like the combination of FastCGI and nginx might provide.

## Using Proxygen

To use Proxygen when running HHVM in server mode:

```
hhvm -m server -p 8080
```

Your port can be whatever you want, of course, via either the following command line configuration setting that you would append to the command above: `-d hhvm.server.port=7777`, or putting `hhvm.server.port=7777` in your `server.ini` file.

Since Proxygen is the default, you don't need to explicitly specify it as the server type, but you could, for verboseness, append the following to the command above as well: `-d hhvm.server.type=proxygen`.

## Example Proxygen Configuration

While not as configurable as a FastCGI/nginx combination, Proxygen does provide sensible defaults for many applications. Thus the simple Proxygen startup sequence above will be just fine.

However, here is an example of some possible configuration options that you could also add/change to your `server.ini` or as `-d` options at the command line:

```
; some of these are not necessary since they are the default value, but
; they are good to show for illustration, and sometimes it is good for
; documentation purposes to be explicit anyway.
; hhvm.server.source_root and hhvm.server.port are the most likely ones
; that need explicit values.
hhvm.server.port = 80
hhvm.server.type = proxygen
hhvm.server.default_document = index.php
hhvm.server.error_document404 = index.php
; default is the current directory where you launched the HHVM binary
hhvm.server.source_root=/var/www/public
```

## Automatic Service Startup

The HHVM Debian prebuilt packages ship with init scripts that start in FastCGI mode by default, so if you want to automatically start HHVM as a service, you need to do some configuration tweaking. Note that this setup is optional; you can manually run HHVM as above, and it will work just fine.

The configuration we need to edit is in `/etc/hhvm/server.ini`. We first need to remove the following line which is in that file by default:

```
hhvm.server.type = fastcgi
```

We also need to add a line that looks like this, to tell HHVM where our code is. Replace `/var/www` with your code's location, of course:

```
hhvm.server.source_root = /var/www
```

You may also want to change `hhvm.server.port` option; it's set to `9000` by default, but `80` or `8080` makes more sense. Finally, note the value of `hhvm.log.file`, which is where error messages will go. It's set to `/var/log/hhvm/error.log` by default, which is just fine unless you'd rather they go elsewhere.

Then, you can run these commands to set HHVM to start up at boot, and to start it as a service now:

```
sudo update-rc.d hhvm defaults
sudo service hhvm restart
```
