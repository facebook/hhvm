# Mcrouter [![Build Status](https://github.com/facebook/mcrouter/workflows/build/badge.svg)](https://github.com/facebook/mcrouter/actions?workflow=build)
[![Support Ukraine](https://img.shields.io/badge/Support-Ukraine-FFD500?style=flat&labelColor=005BBB)](https://opensource.fb.com/support-ukraine) [![License](https://img.shields.io/badge/license-MIT-blue)](https://github.com/facebook/mcrouter/blob/master/LICENSE)

Mcrouter (pronounced mc router) is a memcached protocol router for scaling [memcached](https://memcached.org/)
deployments. It's a core component of cache
infrastructure at Facebook and Instagram where mcrouter handles almost
5 billion requests per second at peak.

Mcrouter is developed and maintained by Facebook.

See https://github.com/facebook/mcrouter/wiki to get started.

## Quick start guide

### New! Ubuntu package available

Currently, we support Ubuntu Bionic (18.04) amd64.
Here is how to install it:

Add the repo key:

    $ wget -O - https://facebook.github.io/mcrouter/debrepo/bionic/PUBLIC.KEY | sudo apt-key add

Add the following line to apt sources file /etc/apt/sources.list

    deb https://facebook.github.io/mcrouter/debrepo/bionic bionic contrib

Update the local repo cache:

    $ sudo apt-get update

Install mcrouter:

    $ sudo apt-get install mcrouter


### Installing From Source

See https://github.com/facebook/mcrouter/wiki/mcrouter-installation for more
detailed installation instructions.

Mcrouter depends on [folly](https://github.com/facebook/folly), [wangle](https://github.com/facebook/wangle), [fizz](https://github.com/facebookincubator/fizz), and [fbthrift](https://github.com/facebook/fbthrift).

The installation is a standard autotools flow:

    $ autoreconf --install
    $ ./configure
    $ make
    $ sudo make install
    $ mcrouter --help

Assuming you have a memcached instance on the local host running on port 5001,
the simplest mcrouter setup is:

    $ mcrouter \
        --config-str='{"pools":{"A":{"servers":["127.0.0.1:5001"]}},
                      "route":"PoolRoute|A"}' \
        -p 5000
    $ echo -ne "get key\r\n" | nc 0 5000

(nc is the GNU Netcat, http://netcat.sourceforge.net/)

## Features

+ Memcached ASCII protocol
+ Connection pooling
+ Multiple hashing schemes
+ Prefix routing
+ Replicated pools
+ Production traffic shadowing
+ Online reconfiguration
+ Flexible routing
+ Destination health monitoring/automatic failover
+ Cold cache warm up
+ Broadcast operations
+ Reliable delete stream
+ Multi-cluster support
+ Rich stats and debug commands
+ Quality of service
+ Large values
+ Multi-level caches
+ IPv6 support
+ SSL support

## Links

Documentation: https://github.com/facebook/mcrouter/wiki
Engineering discussions and support: https://www.facebook.com/groups/mcrouter

## License

Copyright (c) Facebook, Inc. and its affiliates.

Licensed under the MIT license:
https://github.com/facebook/mcrouter/blob/master/LICENSE
