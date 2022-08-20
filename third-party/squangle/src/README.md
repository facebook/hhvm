SQuangLe
========

Overview
--------

SQuangLe is a C++ MySQL client library built on top of
[WebScaleSQL](http://webscalesql.org/)'s C client library. It does not require
a WebScaleSQL server - WebScaleSQL, MySQL, MariaDB, and Percona Server should
all work fine.

Current Status
--------------

SQuangLe is not supported as a standalone project; it is released as a
dependency of HHVM's async mysql extension to the Hack language.

Features
--------

 - Object-oriented API
 - Query builder with automatic escaping
 - Asynchronous query execution

License
-------

SQuangLe is BSD-licensed. We also provide an additional patent grant. Please
see the LICENSE file.

Contributing
------------

Please see CONTRIBUTING.md
