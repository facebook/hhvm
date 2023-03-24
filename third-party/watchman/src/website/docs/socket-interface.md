---
title: Socket Interface
section: Invocation
---

Most simple uses of Watchman will invoke the watchman binary and process its
output.  Sometimes it is desirable to avoid the overhead of an extra process
and talk directly to your watchman service.

The watchman service runs as a single long-lived process per user.  The
watchman binary will take care of spawning the server process if necessary.

The server will create a unix domain socket for communication with its clients.
The location of the socket depends on compile time options and command line
flags.  It is recommended that you invoke `watchman get-sockname` to discover
the location, or if you are being invoked via a trigger (since version 2.9.7)
you will find the location in the `$WATCHMAN_SOCK` environmental variable.

## Watchman Protocol

The unix socket implements a request-response protocol with PDUs encoded in
either JSON or BSER representation.  Some watchman commands (notably
`subscribe` and `log-level`) allow the watchman service to unilaterally send
any number of PDUs to the client, and require more stateful handling.

### JSON encoding

The JSON encoding represents a request or a response as a single line of
compact JSON encoded data.  The newline is used to detect the end of the PDU.

Requests from the client are always represented as a JSON array.

Responses from the server are always represented as a JSON object.

Sending the `since` command is simply a matter of formatting it as JSON.  Note
that the JSON text must be a single line (don't send a pretty printed version
of it!) and be followed by a newline `\n` character:

~~~json
["since", "/path/to/src", "n:c_srcs", "*.c"] <NEWLINE>
~~~

### BSER encoding

BSER is a local-only binary serialization format that can represent the same
data types as JSON, but in a more compact form and not be limited to UTF-8
representation of strings.

When you make a request using BSER, the server will respond in BSER encoding.

You can [read more about BSER in the BSER specification](
/watchman/docs/bser.html).

### Reporting Errors and Warnings

If a Response includes a field named `error` it indicates that the request was
not successful.  The `error` field, if present, is a string value that can be
presented to the user.

If a Response includes a field named `warning` it indicates that there is some
condition that should be reported back to the user so that the system can
operate more effectively.  Tools should display this to the user as an
advisory.  At the time of writing, this can be triggered due to overflowing the
system limits on file watching resources.  These are important to address so
that events are not missed and thus so that correctness and system performance
are maintained.

The warning message contains a link to this documentation that provides
advice on tuning and resolving the issue.
