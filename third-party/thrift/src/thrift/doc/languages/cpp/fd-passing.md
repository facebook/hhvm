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

## How to pass FDs over Thrift

For a client to pass FDs to a server, we use the following very temporary hack. This will be improved shortly:

```
apache::thrift::RpcOptions rpcOptions;
rpcOptions.setWriteHeader("__UNSAFE_FDS_FOR_REQUEST__", "FD1,FD2");
client.header_semifuture_METHOD(rpcOptions, ...)...
```

On the server handler, FDs can be received and sent via `THeader::fds`:

```
folly::coro::Task<...> co_METHOD(
    apache::thrift::RequestParams params, ...) override {
  auto inFds = reqCtx->getHeader()->fds.releaseReceived();
  ...
  folly::SocketFds::ToSend outFds;
  ...
  reqCtx->getHeader()->fds.dcheckEmpty() = folly::SocketFds{std::move(outFds)};
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
