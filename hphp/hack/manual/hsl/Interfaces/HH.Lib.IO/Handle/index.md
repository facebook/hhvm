---
title: Handle
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An interface for an IO stream




For example, an IO handle might be attached to a file, a network socket, or
just an in-memory buffer.




HSL IO handles can be thought of as having a combination of behaviors - some
of which are mutually exclusive - which are reflected in more-specific
interfaces; for example:

+ Closeable
+ Seekable
+ Readable
+ Writable




These can be combined to arbitrary interfaces; for example, if you are
writing a function that writes some data, you may want to take a
[` IO\WriteHandle `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/>) - or, if you read, write, and seek,
[` IO\SeekableReadWriteHandle `](</hsl/Interfaces/HH.Lib.IO/SeekableReadWriteHandle/>); only specify `` Closeable `` if
your code requires that the close method is defined.




Some types of handle imply these behaviors; for example, all [` File\Handle `](</hsl/Interfaces/HH.Lib.File/Handle/>)s
are [` IO\SeekableHandle `](</hsl/Interfaces/HH.Lib.IO/SeekableHandle/>)s.




You probably want to start with one of:

* ` File\open_read_only() `, `` File\open_write_only() ``, or
  ``` File\open_read_write() ```
* ` IO\pipe() `
* ` IO\request_input() `, `` IO\request_output() ``, or ``` IO\request_error() ```; these
  used for all kinds of requests, including both HTTP and CLI requests.
* ` IO\server_output() `, `` IO\server_error() ``
* ` TCP\connect_async() ` or [` TCP\Server `](/hsl/Classes/HH.Lib.TCP/Server/)
* ` Unix\connect_async() `, or [` Unix\Server `](/hsl/Classes/HH.Lib.Unix/Server/)




## Interface Synopsis




``` Hack
namespace HH\Lib\IO;

interface Handle {...}
```



<!-- HHAPIDOC -->
