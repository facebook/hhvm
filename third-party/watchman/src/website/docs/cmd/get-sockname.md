---
title: get-sockname
section: Commands
---

If you're integrating against watchman using the unix socket and either the
JSON or BSER protocol, you may need to discover the correct socket path.
Rather than hard-coding the path or replicating the logic discussed in
[Command Line](/watchman/docs/cli-options.html), you can simply execute the
CLI to determine the path.  This has the side effect of spawning the service
for your user if it was not already running--bonus!

~~~bash
$ watchman get-sockname
{
  "version": "2.5",
  "sockname": "/tmp/.watchman.wez"
}
~~~
