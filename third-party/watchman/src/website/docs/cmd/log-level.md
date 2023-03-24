---
pageid: cmd.log-level
title: log-level
layout: docs
section: Commands
permalink: docs/cmd/log-level.html
redirect_from: docs/cmd/log-level/
---

Changes the log level of your connection to the watchman service.

From the command line:

~~~bash
$ watchman --server-encoding=json --persistent log-level debug
~~~

JSON:

~~~json
["log-level", "debug"]
~~~

This command changes the log level of your client session.  Whenever watchman
writes to its log, it walks the list of client sessions and also sends a log
packet to any that have their log level set to match the current log event.

Valid log levels are:

 * `debug` - receive all log events
 * `error` - receive only important log events
 * `off`   - receive no log events

Note that you cannot tap into the output of triggered processes using this
mechanism.

Log events are sent unilaterally by the server as they happen, and have
the following structure:

~~~json
{
  "version": "1.0",
  "log": "log this please"
}
~~~
