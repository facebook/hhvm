# Admin Server

The admin server allows the administrator of the HHVM server to query and control the HHVM server process. It is different and separate than the primary HHVM server that you specified with `-m server` or `-m daemon`.

To turn on the admin server, you specify the following options at the command line via `-d` or within your `server.ini` (or equivalent).

```
hhvm.admin_server.port=9001
hhvm.admin_server.password=SomePassword
```

The `port` can be any open port. And you should **always specify a password** to secure the admin port since you don't want just anybody being able to control your server. In fact, you will probably want to put the admin server *behind a firewall*. You will specify the password with every request to the admin port.

The admin server uses the same protocol as the main server - so, if you're using [FastCGI](/hhvm/advanced-usage/fastCGI) mode, the admin server will also be FastCGI, and you will need to configure a front-end webserver (like nginx). If you are using [Proxygen](/hhvm/basic-usage/proxygen) mode, the admin server will be an HTTP server.

## Querying the Admin Server

Once you have set up your admin server, you can query it via `curl`.

```
curl http://localhost:9001/
```

will bring up a list of commands you can use to control and query your admin server.

The port associated with the `curl` command is the `hhvm.admin_server` port set above if you are using [Proxygen](/hhvm/basic-usage/proxygen). *If you are using [FastCGI](/hhvm/advanced-usage/fastCGI)*, then the port will be the webserver port that is the front end to FastCGI.

### Sending a Command

Use one of the commands listed with the `curl` sequence above, along with your password, to send a command to the admin server.

```
curl http://localhost:9001/compiler-id?auth=SomePassword
```

## Further Reference

There is a good [blog post](http://hhvm.com/blog/521/the-adminserver) discussing the admin server even further.
