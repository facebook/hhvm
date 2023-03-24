---
title: state-leave
section: Commands
---

*Since 4.4*

The `state-leave` command works in conjunction with
[state-enter](/watchman/docs/cmd/state-enter.html) to facilitate [advanced
settling in subscriptions](/watchman/docs/cmd/subscribe.html#advanced-settling).

`state-leave` causes a watch to no longer be marked as being in a particular
named state.

### Examples

This is the simplest example, vacating a state named `mystate`:

~~~json
["state-leave", "/path/to/root", "mystate"]
~~~

It will cause any subscribers to receive a unilateral subscription PDU from the
watchman server:

~~~json
{
  "subscription":  "mysubscriptionname",
  "root":          "/path/to/root",
  "state-leave":   "mystate",
  "clock":         "c:1446410081:18462:7:135"
}
~~~

The `clock` field in the response is the (synchronized; see below) clock at the
time that the state was entered and can be used in subsequent queries, in
combination with the corresponding `state-enter` subscription PDU clock, to
locate things that changed while the state was asserted.

A more complex example demonstrates passing metadata to any subscribers.  The
`metadata` field is propagated through to the subscribers and is not
interpreted by the watchman server.  It can be any JSON value.

~~~json
["state-leave", "/path/to/root", {
  "name": "mystate",
  "metadata": {
    "foo": "bar"
  }
}]
~~~

This will emit the following unilateral subscription PDU to all subscribers:

~~~json
{
  "subscription":  "mysubscriptionname",
  "root":          "/path/to/root",
  "state-leave":   "mystate",
  "clock":         "c:1446410081:18462:7:137",
  "metadata": {
    "foo": "bar"
  }
}
~~~

### Abandoned State

A state is implicitly vacated when the watchman client session that asserted it
disconnects.  This helps to avoid breaking subscribers (since the typical
action is to defer or drop notifications) if the tooling that initiated the
state terminates unexpectedly.

An abandoned state is reported to any subscribers via a unilateral subscription
PDU with the `abandoned` field set to `true`:

~~~json
{
  "subscription":  "mysubscriptionname",
  "root":          "/path/to/root",
  "state-leave":   "mystate",
  "clock":         "c:1446410081:18462:7:137",
  "abandoned":     true
}
~~~

This allows the subscriber to take an appropriate action.

### Synchronization

States are synchronized with the state of the filesystem so that it is
possible for subscribers to reason about when files changed with respect to
the state.

This means that issuing a `state-leave` command will [perform query
synchronization](/watchman/docs/cookies.html#how-cookies-work) to ensure that
things are in sync.

The `state-leave` command will use a default `sync_timeout` of 60 seconds.
If the synchronization cookie is not observed within the configured
`sync_timeout`, an error will be returned and *the state will not be entered*.

In some cases, perhaps during the initial crawl of a very large tree, You may
specify an alternative value for the timeout; the value is expressed in
*milliseconds*:

~~~json
["state-leave", "/path/to/root", {
  "name": "mystate",
  "sync_timeout": 10000,
  "metadata": {
    "foo": "bar"
  }
}]
~~~

You may also specify `0` for the timeout to disable synchronization for this
particular command.   This may cause the state to appear to clients to have
been vacated logically before it actually did in the case that there are
buffered notifications that have not yet been processed by watchman at the time
that the state was vacated.
