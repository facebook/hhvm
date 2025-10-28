# Introduction

Normally, after installing HHVM, you can use the [defaults](/docs/hhvm/basic-usage/introduction) provided to [run Hack code](/docs/hhvm/basic-usage/command-line) or [run HHVM as a server](/docs/hhvm/basic-usage/server).

While a majority of the time you will not need to tweak the default settings or use the more advanced modes available with HHVM, they are available:

* [Repo Authoritative](/docs/hhvm/advanced-usage/repo-authoritative) mode allows you to compile your entire codebase into one unit, allowing for HHVM to perform highly aggressive optimizations to make your code run quickly.
* [Daemon](/docs/hhvm/advanced-usage/daemon) mode allows you to run HHVM as a background process.
* The [admin server](/docs/hhvm/advanced-usage/admin-server) allows you to monitor HHVM as it is running in server mode.
* [FastCGI](/docs/hhvm/advanced-usage/fastCGI) is another server type for HHVM that is highly configurable and fast, but requires a separate web server on top of it.

## Custom Configuration

There are also a plethora of custom [configuration options](/docs/hhvm/configuration/introduction) that you can set to tweak how HHVM operates when running scripts or running as a server.
