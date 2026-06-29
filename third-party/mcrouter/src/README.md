# Mcrouter [![Build Status](https://github.com/facebook/mcrouter/workflows/build/badge.svg)](https://github.com/facebook/mcrouter/actions?workflow=build)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://github.com/facebook/mcrouter/blob/master/LICENSE)

Mcrouter (pronounced mc router) is a memcached protocol router for scaling [memcached](https://memcached.org/)
deployments. It's a core component of cache
infrastructure at Facebook and Instagram where mcrouter handles almost
5 billion requests per second at peak.

Mcrouter is developed and maintained by Facebook.

See https://github.com/facebook/mcrouter/wiki to get started.

## Quick start guide

### Installing From Source
Mcrouter depends on [folly](https://github.com/facebook/folly), [wangle](https://github.com/facebook/wangle), [fizz](https://github.com/facebookincubator/fizz), [mvfst](https://github.com/facebook/mvfst), and [fbthrift](https://github.com/facebook/fbthrift).

Mcrouter is built using CMake. It's recommended to invoke it via `getdeps` to simplify the process of building and installing dependencies:

    # Optionally install system dependencies first.
    $ ./build/fbcode_builder/getdeps.py install-system-deps --recursive mcrouter
    # Build, preferring to use system dependencies if available.
    $ ./build/fbcode_builder/getdeps.py build --allow-system-packages mcrouter

`getdeps.py` will invoke cmake etc and put output in its scratch area (you can see in logs, and can override with `--scratch-path`):

* `installed/mcrouter/bin/mcrouter`: The standalone mcrouter executable.
* `installed/mcrouter/include`: libmcrouter development headers.
* `installed/mcrouter/lib`: CMake targets and libraries for applications embedding libmcrouter.

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
