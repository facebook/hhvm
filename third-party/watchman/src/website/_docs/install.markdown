---
pageid: install
title: Installation
layout: docs
section: Installation
permalink: docs/install.html
redirect_from: docs/install/index.html
---

## System Requirements

Watchman is known to compile and pass its test suite on:

 * <i class="fa fa-linux"></i> Linux systems with `inotify`
 * <i class="fa fa-apple"></i> macOS (uses `FSEvents` on 10.7+,
   `kqueue(2)` on earlier versions)
 * <i class="fa fa-windows"></i> Windows x64 on Windows 7,
   Windows Server 2012 R2 and later is currently in **beta** status.

Watchman used to support the following systems, but no one is actively
maintaining them.  The core of the code should be OK, but they likely don't
build.  We'd love it if someone would step forward to maintain them:

 * BSDish systems (FreeBSD 9.1, OpenBSD 5.2) that have the
   `kqueue(2)` facility
 * Illumos and Solaris style systems that have `port_create(3C)`

Watchman relies on the operating system facilities for file notification,
which means that you will likely have very poor results using it on any
kind of remote or distributed filesystem.

Watchman does not currently support any other operating system not covered by
the list above.

## Download for Windows (Beta)

Watchman is considered to be in **beta** status for Windows but has
a reasonably sized group of users depending on it already, and we expect
to remove the beta label in the coming months.

Watchman was built to support Windows Server 2012 R2 and later, but has
had community-provided patches that enable support for Windows 7 and later.

At this time, we recommend running the latest build of watchman on Windows; see
below for binary downloads!

If you encounter issues with the Windows version of watchman, please report
them via GitHub!  [You can find the list of known Windows issues here](
https://github.com/facebook/watchman/issues?utf8=%E2%9C%93&q=is%3Aopen+Windows).

## Binary Downloads for Linux, macOS and Windows (Beta)

*Note: binaries are built from the main branch only.  We don't provide binaries for v4.9.0*

Watchman is continuously deployed as it passes our internal test validation
inside Facebook and doesn't use manually assigned or "approved" version numbers.

Outside FB we have automation that cuts a tag and builds binaries on Monday of
each week and assigns a tag based on the date.  That process is in a beta
state; some or all of the binaries may not be present for any given tag.

You can find the binary downloads in the [latest release](https://github.com/facebook/watchman/releases/latest)

### Windows

* Download and extract the windows release from the [latest release](https://github.com/facebook/watchman/releases/latest)
* It will be named something like `watchman-vYYYY.MM.DD.00-windows.zip`
* It contains a `bin` folder.  Move that somewhere appropriate and update your `PATH` environment to reference that location.

### Linux and macOS

* Download and extract the release for your system from the [latest release](https://github.com/facebook/watchman/releases/latest)
* It will be named something like `watchman-vYYYY.MM.DD.00-linux.zip`

~~~bash
$ unzip watchman-*-linux.zip
$ cd watchman-vYYYY.MM.DD.00-linux
$ sudo mkdir -p /usr/local/{bin,lib} /usr/local/var/run/watchman
$ sudo cp bin/* /usr/local/bin
$ sudo cp lib/* /usr/local/lib
$ sudo chmod 755 /usr/local/bin/watchman
$ sudo chmod 2777 /usr/local/var/run/watchman
~~~

## Build/Install

### Installing on macOS or Linux via Homebrew

To build the most recent release currently tracked by
[Homebrew](http://brew.sh/):

~~~bash
$ brew update
$ brew install watchman
~~~

To install the latest build from github:

~~~bash
$ brew install --HEAD watchman
~~~

### Installing on macOS via macports

To install the package maintained by [MacPorts](https://www.macports.org):

~~~bash
$ sudo port install watchman
~~~

### Installing on Windows via Chocolatey

Watchman is available to install via the [Chocolatey](https://www.chocolatey.org) Windows package manager.  Installation is as simple as:

~~~powershell
PS C:\> choco install watchman
~~~

The package is maintained by the community rather than by Facebook, Inc., so if you experience issues with installation/uninstallation, you should [contact the package maintainers](https://chocolatey.org/packages/watchman/ContactOwners) for assistance.

### Installing from source

You can use these steps below to get watchman built.  You will need `libssl-dev`,
`autoconf`, `automake` and `libtool` (or `glibtool` on macOS).

You may optionally build watchman without `pcre` and `python` support (see
configuration options below).

For python support, you will need `setuptools` and may need to install a
`python-dev` or `python-devel` package. To build the C++ client library you will
need to install `libfolly`.

See below for some more information on options to configure your build.

~~~bash
$ # use the latest stable release
$ git clone https://github.com/facebook/watchman.git -b {{ site.data.current_release.tag }} --depth 1
$ cd watchman
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
~~~

### Compile Time Configuration Options

Our configure script accepts all the standard options, but there are a couple
that are specific to watchman that might be relevant to your needs:

~~~
--enable-conffile=PATH  Use PATH as the default configuration file name.
                        Default is /etc/watchman.json

--enable-statedir=PATH  Use PATH as the default for state, log files
                        and sockets instead of using your system tempdir

--enable-lenient  Turn off more pedantic levels of warnings
                  and compilation checks

--enable-stack-protector  Enable stack protection in the same
                          way that rpmbuild does on some systems.

--enable-cppclient  Enable build of the C++ client library. This is built by
                    default if Folly is available.

--with-buildinfo=TEXT   Include some extra build information that will
                        be reported in the version command output

--without-python        Disable python bindings
--with-python=PATH      Enable Python bindings. PATH is location of python.
                        Default is to look for python in your PATH

--without-pcre       Don't enable pcre support.
--with-pcre=PATH     Enable pcre support.  PATH is location of pcre-config.
                     Default is to enable and look for pcre-config in your
                     $PATH

--with-folly=PATH  Specify root for Folly (needed for the C++ client library)
                   https://github.com/facebook/folly
~~~

(Run `./configure --help` to get the list for the version you checked out)

## System Specific Preparation

### Linux inotify Limits

The `inotify(7)` subsystem has three important tunings that impact watchman.

 * `/proc/sys/fs/inotify/max_user_instances` impacts how many different
   root dirs you can watch.
 * `/proc/sys/fs/inotify/max_user_watches` impacts how many dirs you
   can watch across all watched roots.
 * `/proc/sys/fs/inotify/max_queued_events` impacts how likely it is that
   your system will experience a notification overflow.

You obviously need to ensure that `max_user_instances` and `max_user_watches`
are set so that the system is capable of keeping track of your files.

`max_queued_events` is important to size correctly; if it is too small, the
kernel will drop events and watchman won't be able to report on them.  Making
this value bigger reduces the risk of this happening.

Watchman has two simple strategies for mitigating an overflow of
`max_queued_events`:

 * It uses a dedicated thread to consume kernel events as quickly as possible
 * When the kernel reports an overflow, watchman will assume that all the files
   have been modified and will re-crawl the directory tree as though it had just
   started watching the dir.

This means that if an overflow does occur, you won't miss a legitimate change
notification, but instead will get spurious notifications for files that
haven't actually changed.

### Mac OS File Descriptor Limits

*Only applicable on macOS 10.6 and earlier*

The default per-process descriptor limit on macOS is extremely low (256!).

Watchman will attempt to raise its descriptor limit to match
`kern.maxfilesperproc` when it starts up, so you shouldn't need to mess with
`ulimit`; just raising the sysctl should do the trick.

The following will raise the limits to allow 10 million files total, with 1
million files per process until your next reboot.

~~~bash
$ sudo sysctl -w kern.maxfiles=10485760
$ sudo sysctl -w kern.maxfilesperproc=1048576
~~~

Putting the following into a file named `/etc/sysctl.conf` on macOS will cause
these values to persist across reboots:

~~~
kern.maxfiles=10485760
kern.maxfilesperproc=1048576
~~~
