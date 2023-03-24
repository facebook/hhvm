---
title: subscribe
category: Commands
---

_Since 1.6_

Subscribes to changes against a specified root and requests that they be sent to
the client via its connection. The updates will continue to be sent while the
connection is open. If the connection is closed, the subscription is implicitly
removed.

This makes the most sense in an application connecting via the socket interface,
but you may also subscribe via the command line tool if you're interested in
observing the changes for yourself:

```bash
$ watchman -j --server-encoding=json -p <<-EOT
["subscribe", "/path/to/root", "mysubscriptionname", {
  "expression": ["allof",
    ["type", "f"],
    ["not", "empty"],
    ["suffix", "php"]
  ],
  "fields": ["name"]
}]
EOT
```

The example above registers a subscription against the specified root with the
name `mysubscriptionname`.

The response to a subscribe command looks like this:

```json
{
  "version":   "1.6",
  "subscribe": "mysubscriptionname"
}
```

When the subscription is first established, the expression term is evaluated and
if any files match, a subscription notification packet is generated and sent,
unilaterally to the client.

Then, each time a change is observed, and after the settle period has passed,
the expression is evaluated again. If any files are matched, the server will
unilaterally send the query results to the client with a packet that looks like
this:

```json
{
  "version": "1.6",
  "clock": "c:1234:123",
  "files": ["one.php"],
  "root":  "/path/being/watched",
  "subscription": "mysubscriptionname"
}
```

The subscribe command object allows the client to specify a since parameter; if
present in the command, the initial set of subscription results will only
include files that changed since the specified clockspec, equivalent to using
the `query` command with the `since` generator.

```json
["subscribe", "/path/to/root", "myname", {
  "since": "c:1234:123",
  "expression": ["not", "empty"],
  "fields": ["name"]
}]
```

The suggested mode of operation is for the client process to maintain its own
local copy of the last "clock" value and use that to establish the subscription
when it first connects.

## Filesystem Settling

Prior to watchman version 3.2, the settling behavior was to hold subscription
notifications until the kernel notification stream was complete.

Starting in watchman version 3.2, after the notification stream is complete, if
the root appears to be a version control directory, subscription notifications
will be held until an outstanding version control operation is complete (at the
time of writing, this is based on the presence of either `.hg/wlock` or
`.git/index.lock`). This behavior matches triggers and helps to avoid performing
transient work in response to files changing, for example, during a rebase
operation.

In some circumstances it is desirable for a client to observe the creation of
the control files at the start of a version control operation. You may specify
that you want this behavior by passing the `defer_vcs` flag to your subscription
command invocation:

```bash
$ watchman -j -p <<-EOT
["subscribe", "/path/to/root", "mysubscriptionname", {
  "expression": ["allof",
    ["type", "f"],
    ["not", "empty"],
    ["suffix", "php"]
  ],
  "defer_vcs": false,
  "fields": ["name"]
}]
EOT
```

## Advanced Settling

_Since 4.4_

In more complex integrations it is desirable to be able to have a watchman aware
application signal the beginning and end of some work that will generate a lot
of change notifications. For example, Mercurial or Git could communicate with
watchman before and after updating the working copy.

Some applications will want to know that the update is in progress and continue
to process notifications. Others may want to defer processing the notifications
until the update completes, and some may wish to drop any notifications produced
while the update was in progress.

Watchman subscriptions provide the mechanism for each of these use cases and
expose it via two new fields in the subscription object; `defer` and `drop` are
described below.

It can be difficult to mix `defer` and `drop` with multiple overlapping states
in the context of a given subscription stream as there is a single cursor to
track the subscription position.

If your application uses multiple overlapping states and wants to `defer` some
results and `drop` others, it is recommended that you use `drop` for all of the
states and then issues queries with `since` terms bounded by the `clock` fields
from the subscription state PDUs to ensure that it observes all of the results
of interest.

### defer

```json
["subscribe", "/path/to/root", "mysubscriptionname", {
  "defer": ["mystatename"],
  "fields": ["name"]
}]
```

The `defer` field specifies a list of state names for which the subscriber
wishes to defer the notification stream. When a watchman client signals that a
state has been entered via the
[state-enter](/watchman/docs/cmd/state-enter.html) command, if the state name
matches any in the `defer` list then the subscription will emit a unilateral
subscription PDU like this:

```json
{
  "subscription":  "mysubscriptionname",
  "root":          "/path/to/root",
  "state-enter":   "mystatename",
  "clock":         "<clock>",
  "metadata":      <metadata from the state-enter command>
}
```

Watchman will then defer sending any subscription PDUs with `files` payloads
until the state is vacated either by a
[state-leave](/watchman/docs/cmd/state-leave.html) command or by the client that
entered the state disconnecting from the watchman service.

Once the state is vacated, watchman will emit a unilateral subscription PDU like
this:

```json
{
  "subscription":  "mysubscriptionname",
  "root":          "/path/to/root",
  "state-leave":   "mystatename",
  "clock":         "<clock>",
  "metadata":      <metadata from the exit-state command>
}
```

The subscription stream will then be re-enabled and notifications received since
the corresponding `state-enter` will be delivered to clients.

### drop

```json
["subscribe", "/path/to/root", "mysubscriptionname", {
  "drop": ["mystatename"],
  "fields": ["name"]
}]
```

The `drop` field specifies a list of state names for which the subscriber wishes
to discard the notification stream. It works very much like `defer` as described
above, but when a state is vacated, the pending notification stream is
fast-forwarded to the clock of the `state-leave` command, effectively
suppressing any notifications that were generated between the `state-enter` and
the `state-leave` commands.

## Source Control Aware Subscriptions

_Since 4.9_

[Read more about these here](/watchman/docs/scm-query.html)
