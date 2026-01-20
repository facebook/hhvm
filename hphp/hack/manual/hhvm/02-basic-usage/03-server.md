# Server

Use HHVM server mode to create a HHVM process that continuously serves web requests, with these advantages:

- Multiple requests served simultaneously
- HHVM caches code to be shared across requests

## Quick Start
Here's the simplest way to run HHVM in Server mode.

```
$ hhvm -m server -p 8080
```

- `-m` is the `mode` option; the default is [Command Line mode](/hhvm/basic-usage/command-line).
- `-p` is the port HHVM uses to listen for requests. The default is `80`.

Other things to know:
- The root for your program files is the directory that you used to launch the `hhvm` command.
- By default, HHVM uses the built-in [proxygen](/hhvm/basic-usage/proxygen) web server.

## Configuration Overrides
Use the `-d` option to override [configuration](/hhvm/configuration/introduction) defaults and [other options](/hhvm/configuration/INI-settings).

In our earlier example, we started a HHVM server with `-p 8080`, but you could also have set the port with its expanded property:

```
$ hhvm -m server -d hhvm.server.port=7777
```

And we also could have overridden other defaults like the [server type](/hhvm/basic-usage/proxygen) or source root of your project files. For example:

```
$ hhvm --mode server -d hhvm.server.type=${SERVER_TYPE} -d hhvm.server.source_root=${PROJECT_FOLDER}
```

### INI Configuration Values
HHVM uses the default [INI configuration](/hhvm/configuration/INI-settings) specified in `server.ini`.

The default ini locations are:
- Linux: `/etc/hhvm/`
- MacOS: `/usr/local/etc/hhvm/`

## Client access to HHVM in Server mode
Normally, a web request of the form:

```
http://your.site:8080/index.hack
```

You can also use `curl` and other programs to access the HHVM server as well.
