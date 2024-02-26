---
state: experimental
---

# Thrift FD passing via Unix sockets

:::note

Thrift FD passing is an experimental feature that is only implemented in C++.

:::

## What is FD passing?

Historically, Thrift has focused on communication with remote hosts over TCP connections. On-host RPC would benefit equally from Thrift’s IDL-to-polyglot code generation, high performance, and mature language support libraries. However, same-host communication has some additional demands, having to do with sharing kernel resources. Specifically, Unix domain sockets (`man unix`) support so-called ancillary data (`man cmsg`), which allows the sender to pass via the socket, alongside regular data bytes, either:

* File descriptors (`SCM_RIGHTS`), with the semantic that the receiver process gets an FD that shares the same open file description item in the kernel as the sender, analogous to the effect of calling `dup` within the same process.
* Process credentials (`SCM_CREDENTIALS`) — this gives the receiver a kernel-verified process, user, and group ID of the sender. This is not the focus of the current document, but could be a Thrift future extension.
  See "How to authenticate peer without `SCM_CREDENTIALS`?" below for alternatives.

## How to pass FDs over Thrift

For a client to pass FDs to a server, use `RpcOptions::setFdsToSend`. To read FDs in a server's response, use the `header_semifuture_` client methods, and look on `THeader::fds`.

```
SocketFds::ToSend fds;
fds.emplace_back(std::make_shared(std::move(file1)));
fds.emplace_back(std::make_shared(std::move(file2)));
apache::thrift::RpcOptions rpcOptions;
rpcOptions.setFdsToSend(std::move(fds));  // Send FDs to server
client.header_semifuture_METHOD(rpcOptions, ...).thenTry([](auto maybeRet) {
  ...
  // Receive FDs from server
  auto [res, header] = std::move(*maybeRet);
  CHECK(!header->fds.empty());
  auto fds = header->fds.releaseReceived();
});
```

On the server handler, FDs can be received and sent via `THeader::fds`:

```
folly::coro::Task<...> co_METHOD(
    apache::thrift::RequestParams params, ...) override {
  auto& headerFds = reqCtx->getHeader()->fds;
  CHECK(!headerFds.empty());
  auto inFds = headerFds.releaseReceived();
  ...
  // Send FDs back to client -- remember, you must `releaseReceive` first.
  folly::SocketFds::ToSend outFds;
  ...
  headerFds.dcheckEmpty() = folly::SocketFds{std::move(outFds)};
}
```

Finally, to handle FDs on stream messages, the client / server must use the library flavor that passes `RichPayloadToSend` / `RichPayloadReceived` structures. These include `fds` both on the “send” (`StreamCallbacks.h`) and “receive” side (`ClientBufferedStream.h`).

The design & implementation of this feature is [documented here](../../contributions/fd-passing.md).

## What Thrift messages can pass FDs today?

Thrift has a variety of request types, we support FDs on most of them:

* Regular request-response,
* Request-only (aka fire-and-forget),
* Stream first request, first response, and the actual streaming payloads.

A few things are not supported, but could be, with some additional work:

* Sinks are currently not supported due to low usage, and lack of `THeader` support in the server handler codegen.
* There was not attempt to support the `sync_` code paths.
* Thrift exceptions cannot yet pass FDs.

Some “Thrift-internal” socket traffic will probably never carry FDs — a good rule of thumb is that if there is no IDL structure being transported, FDs will not be supported. Specifically, FDs will not travel with void return values, internal errors, or internal protocol messages.

## Why should Thrift support FD passing?

The standard for Linux on-host communication is D-Bus (and its very close relative `sd-bus`). Both supports FD-passing, and the feature is used extensively in `systemd` for things like process hotswap, container / unit startup via RPC, etc. D-Bus is straightforward, well-documented, has [excellent introspection](https://0pointer.net/blog/the-new-sd-bus-api-of-systemd.html), and reasonably easy to use. However, for Meta infra applications, D-Bus has some key downsides:

* Not focused on performance or request concurrency — a blocker for several applications.
* No universal codegen from IDL, and much lower level of [client & server support](https://www.softprayog.in/programming/d-bus-tutorial). It is possible to build out both, but fairly expensive.
* Higher learning curve for the typical Meta infra engineer (whereas everyone knows Thrift).

D-Bus has wide adoption and great introspection, but unfortunately in the "build versus buy" calculation above, it looks prohibitively expensive to make D-Bus as good as Thrift at the things that Meta needs (performance, codegen, client & server stack).  We considered some alternatives to "FDs in Thrift" or "D-Bus", and also rejected them:

* Roll a brand-new IPC server that uses `SOCK_SEQPACKET` with data set to sequence numbers, and FDs populated in ancillary data. Reference the sequence numbers in vanilla Thrift over a separate Unix socket. The need for a second socket path can be avoided by side-channeling a single FD from `socketpair` over the initial Thrift socket. In all, the implementation burden here is not huge — comparable to integrating into Thrift, but by being outside Thrift we're signing up to maintain a separate client & server stack, and will never be able to get to "file descriptor" being a first-class type in the Thrift IDL.
* Use the [file handle API](https://man7.org/linux/man-pages/man2/open_by_handle_at.2.html): Downsides: tricky security issues, only works for files on actual filesystems, and only if the filesystem supports it (`btrfs` does). Linux-only.
* Use `pidfd_getfd`: Does not let unprivileged processes receive FDs. Inherently racy, since if the sender closes the FD before the receiver has fetched it, the FD number can be reused, and cause the **wrong** FD to be transmitted. Linux-only.

## How to authenticate peer without `SCM_CREDENTIALS`?

Consider using Thrift certificate identities in lieu of PIDs for
authorization.  This "how to" is beyond the scope of this section, but the
certs are exposed via `Cpp2ConnContext::getPeerCertificate()`.  This
requires StopTLS to be configured by the client and server.

We didn't implement `SCM_CREDENTIALS` because you can already get the PID of
the caller via `Cpp2ConnContext::getPeerEffectiveCreds()`.

**DANGER 1:** It is unfortunate that this is exposed on the connection
context object, and not on `RequestParams` (and may be worth a future
refactor).  The reason being that `RequestParams::getConnectionContext()` is
a raw pointer that can be invalid by the time that your request handler
starts (another defect that is harder to fix).  The root cause is that the
connection is managed by a Thrift IO thread, but your handler is enqueued to
be run by a separate request thread pool -- the raw pointer predates this
split, and made sense back then.  So, the IO thread can tear down the
connection while your request callback is still in the queue.  Read about
Thrift threading models [here](/fb/server/threading-models.md).

Specific code (from a real service!) that could segfault (but probably won't
do so often):

```
folly::coro::Task<std::unique_ptr<MyResponse>>
MyServiceThriftHandler::co_doCredentialAwareThing(
    apache::thrift::RequestParams params,
    std::unique_ptr<MyRequest> request) {
  // ... various setup ...
  auto requestCtx = params.getRequestContext();
  // CAN SEGFAULT HERE:
  auto peerCreds = requestCtx->getConnectionContext()->getPeerEffectiveCreds();
  // ... do work ...
}
```

There are half-measures for prevention, and one unpleasant but "technically
correct" measure you can take:
  - (half-measure) Call `getPeerEffectiveCreds()` as the very first thing in
    your handler, before any suspensions or slow work.
  - (half-measure) Add a comment explaining potential segfaults on this line.
  - (correct, but painful) Add [`thread = 'eb'`
    ](/fb/server/threading-models.md/#eb-threading-model)
    to the annotation of this method, and implement an `async_eb_` handler.
    This runs your handler **on** the IO thread, so it will no longer race
    with connection close.  The downside is that you can't do anything slow
    in your handler, or it'll make your server unresponsive to new IO. So,
    any slow work should be manually delegated to a separate thread-pool
    after grabbing the credentials.

**DANGER 2:** Using PIDs is inherently racy.  If the time window from the
moment that `Cpp2ConnContext` was constructed and called `getsockopts()` to
the time point that you use the PID exceeds a few seconds, the race can be
exploited by a determined adversary, causing you to act against the wrong
process. The problem is PID reuse:
  - PIDs range from 1 through `/proc/sys/kernel/pid_max`, which can be
    configured up to a hard limit of 4,194,304 -- but is just 32,768 on older
    systems.
  - If the target PID crashes, and enough `fork()` calls happen, the same
    PID can be allocated to a new (and potentially attacker-controlled)
    process.  Per "A Comparison of Processes and Threads Creation" (M.
    Sysel 2020), a single `fork()` takes as little as 34 usec on modern
    Intel machines running Linux, which means that PIDs can wrap around in <
    3 minutes.
  - The race-resistant design would be to use pidfds instead of PIDs, but
    Linux doesn't offer socket credentials in FD form natively.  So a better
    redesign of this API would involve changing `getPeerEffectiveCreds()` to
    return a struct with a `folly::Expected<folly::File,
    folly::exception_wrapper>` object for the PID.  The getter
    `PeerCred::queryFromSocket` would fail to populate the PID if the system
    `pid_max` is below 4 million, and set an appropriate error.  Otherwise,
    it would measue a monotonic timer before calling `getsockopt`, call
    `pidfd_open` (TBD: should it set `PIDFD_NONBLOCK`?), and then measure
    the timer again.  If the time spent is less than 1 second, succeed.
    Otherwise retry a number of times before giving up.  Wrapping around 4M
    PIDs in 1 second would require `fork()` to take <= 250ns, which seems
    safe for the foreseeable future.

Switching to pidfds is therefore a fairly straightforward refactor.  While
undertaking it, it would be worth investigating if `getPeerEffectiveCreds()`
could also be moved to `RequestParams` to fix "DANGER 1" in the same change
stack.
