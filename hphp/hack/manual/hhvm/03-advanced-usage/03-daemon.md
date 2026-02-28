# Daemon

You can run HHVM as a daemon (a background process instead of under the explicit control of a user), you just replace `-m server` with `-m daemon`.

For example, here is how to run HHVM in daemon mode with Proxygen as the backend:

```
hhvm -m daemon -d hhvm.server.type=proxygen -d hhvm.server.port=8080
```

Here is how to run HHVM with a custom `server.ini` file using FastCGI:

```
hhvm -m daemon -c server.ini -d hhvm.server.type=fastcgi -d hhvm.server.port=9000
```
