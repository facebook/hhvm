---
title: flush-subscriptions
category: Commands
---

_Since 4.8._

Flushes buffered updates to subscriptions associated with the current session,
guaranteeing that they are up-to-date as of the time Watchman received the
`flush-subscriptions` command.

Subscription updates will be interleaved between the `flush-subscriptions`
request and its response. Once the response has been received, subscriptions are
up-to-date.

This command is designed to be used by interactive programs that have a
background process or daemon maintaining a subscription to Watchman. The typical
pattern is for interactive commands to be forwarded to the process, which calls
`flush-subscriptions` and then processes any subscription updates it received.
This pattern eliminates races with files changed right before the interactive
command.

### Arguments

- `sync_timeout`: Required. The number of milliseconds to wait to observe a
  synchronization cookie. The synchronization cookie is created at the start of
  the `flush-subscriptions` call, and once the cookie is observed, means that
  the OS has sent watchman all the updates till at least the start of the
  `flush-subscriptions` call.
- `subscriptions`: Optional. Which subscriptions to flush. By default this
  flushes all subscriptions associated with this project on this session.

### Examples

Assuming subscriptions `sub1`, `sub2` and `sub3` have been established on this
session, if `sub1` has updates pending, `sub2` is up-to-date and `sub3` is
currently dropping updates:

```json
["flush-subscriptions", "/path/to/root", {"sync_timeout": 1000}]
```

In response, Watchman will first emit a unilateral subscription PDU for `sub1`,
then respond with

```json
{
  "clock": "c:1446410081:18462:7:135",
  "synced": ["sub1"],
  "no_sync_needed": ["sub2"],
  "dropped": ["sub3"]
}
```

To flush updates for some but not all subscriptions associated with this
session:

```json
["flush-subscriptions", "/path/to/root",
  {
    "sync_timeout": 1000,
    "subscriptions": ["sub1", "sub2"]
  }
]
```

### Deferred and Dropped Updates

Subscriptions will typically buffer individual updates until a _settle_ period
has expired. `flush-subscriptions` will force those updates through immediately.

Subscriptions currently deferring updates because of `defer` or `defer_vcs` are
updated immediately, without waiting for the `defer` or `defer_vcs` to end.

Subscriptions currently dropping updates with a `drop` state will not get any
updates. Their names will be returned in the `dropped` field.

### Notes

- `flush-subscriptions` can only be used to flush subscriptions associated with
  the current session.
- A single session can be subscribed to updates from multiple projects at the
  same time. However, `flush-subscriptions` can only flush updates for one
  project at a time.
