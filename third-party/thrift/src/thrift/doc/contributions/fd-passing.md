# Thrift FD passing via Unix sockets (Rocket details)

Refer to the “Features” section of the docs for [motivation and usage notes](../languages/cpp/fd-passing.md).

## Design principles & invariants

These points aim to help answer “why was it done this way?” and to inform future extensions.

### Don't slow down the "no FDs" code paths

Considerable hardware savings result from Thrift-over-TCP being highly efficient. So, as a rule, the implementation wraps FD containers with `unique_ptr` to keep per-request objects small. FD-related work is hidden behind simple "has FDs?" branches, which are intended to be masked by feedback-directed optimization in production services.

### Work within Rocket instead of working around it

We first tried an “out of Thrift core” approach to FD passing, cobbled together from combination of existing callbacks & observers, together with inheritance, and some brand-new observers. Although this resulted in fewer code changes to Thrift, we decided against it — reasons in priority order:

(1) Reliably matching sent FDs to requests required a number of configuration tweaks to Thrift that were brittle and not well-supportable. For example: disabling client & server send batching; turning on the near-unused flag `enable_rocket_connection_observers`; tightly coupling to acceptor, socket, and request lifetimes; exposing information about frame boundaries to a new observer. As a result future Thrift changes would be likely to break FD-passing code.

(2) Several new reasons to favor an "integrated" implementation emerged. Primarily: (a) Thrift is interested in evolving its implementation to allow passing "out of band" data together with regular Thrift-serialized data to /improve/ performance of large transfers, (b) we identified several important use-cases for FD-passing. Notably, (a) is very similar in intent to FD-passing, so building the latter should help with the former.

(3) The "out of core" implementation relied on a harder-to-understand protocol (see appendix) for associating request data with FD-bearing ancillary data.

### FDs are received no later than the last byte of the message

Rocket invokes `onFullFrame` message processing as soon as it has the last byte of the frame that completes the message. If FDs are not available at this point, we end up with a mess: either Rocket has to learn to wait for FDs (no!), or the user code gets a "future" and has to deal with an ugly API (also no!).

### Allow up to `SCM_MAX_FD` per Thrift message

This Linux kernel constant, [equal to 253](https://elixir.bootlin.com/linux/latest/ident/SCM_MAX_FD), limits how many FDs can travel with one `sendmsg` — and FDs must be sent with at least 1 data byte. Some of our applications require passing millions of FDs within a few seconds, so for efficiency we want to allow at least `SCM_MAX_FD` FDs per request. On the other hand, allowing more than `SCM_MAX_FD` FDs per request would require splitting message data over multiple `sendmsg` calls, which adds complexity without dramatically increasing efficiency (syscall are costly). A particularly bad feature of the "split data" approach is that a message can't have more FDs than (num data bytes * `SCM_MAX_FD`). This inequality depends on the serialization code, and so a program that works today can break tomorrow if the serialization becomes more efficient. Uh-oh.

### Rocket tracks FDs through its batching logic

To improve small-request performance, Rocket does send batching. This involves a single `sendmsg` concatenating data from several messages. It is important that — up until the socket level — FDs remain associated with individual messages, rather than with the batch. If we tried to associate FDs with the batch, a few things would go wrong.

Linux can send up to 253 (`SCM_MAX_FD`) FDs per `sendmsg`. If a batch includes many logical messages each with many FDs, and the socket isn't aware of which FDs go with which data, the socket is forced to write the FDs as early as possible to avoid violating the invariant that FDs arrive no later than the last byte of a the corresponding message.

The write pattern for `Msg1Msg2` with 200 FDs per message would look like this: `sendmsg("M", 253 FDs)`, `sendmsg("s", 147 FDs)`, `sendmsg("g1Msg2")`.

So, FD-unaware batching would force us to send FDs as early as possible to avoid running out of data. That, in turn, requires the receiver to buffer a lot of FDs before the relevant messages' data arrive. And buffering many FDs is a design bug, because processes have FD limits (sometimes low, and outside of the programmer's control), and some operations are linear in the # of FDs. So, by doing "FDs-first" sending, the sender can make a receiver with a low FD limit **unable** to receive a certain set of messages. And since, in the Rocket model, the size of the batch is non-deterministically chosen by the sender, it is possible that a receiver will unpredictably run out of FDs — due to factors outside of its control.

Another take on the "FD-unaware batching" issue is that the receiver, being forced to buffer, is effectively unable to do FD flow control by rate-limiting its request processing (and thus indirectly rate-limiting `recvmsg`). It is essentially stuck receiving more and more FDs until the connection fails due to `EMFILE` "Too many open files".

In the current production implementation, we do FD-aware batching, which means that whenever FDs are present, Rocket makes un-batched `writeChainWithFds` calls to the socket. And without FDs, send batching happens as before.

Since Rocket's batching logic is now FD-aware, we could later reduce the number of `sendmsg` calls for FD-passing by adding a batched-write method to `AsyncFdSocket`. In this optimization, Rocket batch tracking would remain the same, but it would pass the whole batch of (data, fd) pairs to the socket, to be sent in the most efficient way possible.

### Rocket will always send FDs with the final frame of a message

This invariant ensures that FDs are always received the moment that Rocket initiates message processing in `onFullFrame`. If FDs could be sent with an arbitrary frame of the message, it would be hard for the receiver to match the FDs to their message. Here's why.

POSIX guarantees that ancillary data is received with the first data byte of the `sendmsg`, and (implicitly) that ancillary data from two different sends are not concatenated for one `recvmsg`. On Linux `SOCK_STREAM` sockets do not provide any tighter guarantees — `recvmsg` can break up `sendmsg` data boundaries any which way, subject to those POSIX constraints. `SOCK_SEQPACKET` would solve this, but it conflicted with the "peeking" transport used to set up TLS, and was likely to cause other hard-to-diagnose issues downstream — in essence, Thrift assumes it can manage the receive buffer size, but `SOCK_SEQPACKET` requires this to be dictated by protocol to avoid data loss.

The [RSocket protocol](https://rsocket.io/) underlying Thrift Rocket does not forbid the interleaving of frames belonging to a one message with frames from another message. So with messages A and B, the transport sequence `Afirst`, `Blast`, `Alast` is allowed — although Rocket does not do this today.

Together, the prior two paragraphs make it tricky for the receiver to identify the message, matching a given set of FDs. A single `recvmsg` can contain frames from multiple messages, and without some transport-level contract there is no telling what is the "first data byte" the FDs belong to. Sending FDs with the final frame of the message removes this ambiguity.

We talked through a number of alternatives, with the most notable discarded approaches being:

* (As documented below in “Appendix: Context-free...”) The socket uses dummy ancillary data to separate messages, and we extract FDs in `handleFrame`. The main problems were conceptual complexity, and the need for `RocketServerConnection` to interact with FDs.
* Assume the socket has an parallel FD queue. Tag each request's metadata with "socket sequence number of first included FD, number of FDs". Doing this would force us to do a number of unpleasant thing:
    * Introduce coupling between "Rocket metadata serialization" and "FD socket".
    * Require the sending socket to either FAIL on attempts to send out-of-serialization order, or to buffer arbitrary amounts of FDs to deal with any Rocket-internal reordering (which is indeed possible even today, thanks to deferred request initialization).
    * Introduce the "FD exhaustion" problem from the previous section on the receiver, since it would have to buffer to cover for any reordering.
    * Linux will send at most 253 FDs per `sendmsg` (and hence per data byte). If we send a lot of FDs with small messages, and the sender significantly reorders "serialization" versus "sending", the receiver can end up FD-starved — it need to handle a data frame whose FDs have not yet arrived.

## Key implementation concerns

### Mechanism for matching FDs with messages

At a high level, the socket has an associated `queue<vector<File>>`. Each messages's `{RequestRpc,ResponseRpc,StreamPayload}Metadata` struct includes `numFds`. Upon receive, Rocket knows whether to pop FDs from the queue, and can assert that the vector has the right size.

The prior section covered the principles / invariants that Rocket has to adhere to make this work correctly. Read those for the details, but at a high level, we write data frames in lockstep with FDs, in such a way that FDs are sent with the last frame of each message. Rocket uses `AsyncFdSocket::writeChainWithFds` to express this data-FD pairing.

### FD ownership: always RAII, shared for send, unique for receive

Concurrent programs must never handle bare FDs, since accidentally closing the FD outside of the thread / coro can trivially cause you to act on the **wrong file**, due to FD reuse — `EBADF` is peanuts by comparison.

Some services that sends FDs will want to continue using the FDs being sent. So, we want shared ownership for sent FDs. Today, this is captured as `vector<shared_ptr<File>>`. This also works nicely with `StreamMultiPublisher`, which is a Thrift gadget for efficiently sending the same stream to many consumers — the multi-publisher "clones" the serialized payloads (FDs included) before passing them down to the clients' individual connections. The alternative to `shared_ptr` is `dup`, but this feels worse because it costs a syscall, and wastes FD count.

Received FDs, on the other hand, have a clear, single owner, so these are `vector<File>`.

Thrift experts will say — but wait, the `*Payload` structures are currently shared among the "send" and "receive" paths. To deal with this, they store a union type `SocketFds`, which is a `unique_ptr` to either `vector<File>` (receive) or `vector<shared_ptr<File>>` (send). It offers runtime checks that an FD payload in a "send" state cannot be used by receive code (or vice-versa). Compile-time safety here would be preferable, but it would require a future refactor to type-tag all the payload structures in Thrift. Incidentally,  having explicit `{Send,Receive}*Payload` structures Thrift would also help new readers navigate the code.

The current FD ownership implementation has one big wart — though a fix is planned. Thrift's current "client send" plumbing passes all the metadata through `RequestRpcMetadata`, which has the unfortunate property of being a serializable Thrift struct — and `folly::File` ownership semantics cannot be serialized. For expediency, we used a temporary workaround of putting a key `**UNSAFE_FDS_FOR_REQUEST**` with string value `<fd1>,<fd2>,<fd3>` into `otherMetadata`. This has no ownership contract, so we'll be adding correct plumbing ASAP.

### FDs must be managed by RAII **immediately** after `recvmsg`

Today, this just means that the socket internals promptly wrap FDs in `folly::File`. This is hugely important because any FDs created by `recvmsg` that leak due to incorrect error handling are leaked forever, and the program will eventually fail due to `EMFILE`. The diffs to scrutinize for this are D43588338 and D43935125.

### Send/receive FDs through `THeader` or `PayloadWithHeader`

As noted in "FD ownership", current client sends have a much wartier mechanism, but I will fix it soon.

For most other scenarios, `THeader::fds()` is a `SocketFds` object that can be used for sends or receives.

Stream messages do not use `THeader`. For those, the client / server code interacts with two *different* `PayloadWithHeader` structs. The server sees `PayloadWithHeader::fdsToSend` while the client `PayloadWithHeader::receiveFds`.

We plan to evaluate how hard it would be to add IDL support for FDs. In that world, the user would add `folly::File`-like fields to Thrift structs to send or receive FDs.

## Appendix: “Context-free” reconstruction of (IOBuf, FD) sequence

This sketches an *alternate* approach by which the receiver can identify which FDs (from ancillary data) go with which Rocket data frame (not even the full message), without having to mark each message header with “the number of included FDs”, and send the FDs with the last frame, as we do in the production solution. This is included as a historical reference. We rejected this approach since it was more complicated than production, and not clearly better.

The main *advantage* of this approach is that — unlike “queue of FD lists + FD count in header” — it only needs a *frame* to be contiguous on the wire. Although Rocket does not interleave messages today, if it were to do this in the future — e.g. req1a, req2a, req1b, req2b, ... — then this approach will still correctly associate FDs with their precise frame. And the frame can then be robustly matched to the message / request via `streamId`.

What follows is a "proof by example" showing that the receiver can unambiguously match IOBufs and FDs under two easy conditions:

* The write path attaches dummy ancillary data to each IOBuf immediately after one that had FDs, forcing "tail separation" of IOBuf on the receiver.
* `RocketServerConnection::handleFrame` knows whether this frame exhausted the read buffer.

Instructions for the "proof":

* Each line shows an example receive stream.
* Semicolon separates `recvmsg`s.
* Pure whitespace separates separately sent IOBufs.
* The repetition of a word like `req1` means that it got split across `recvmsg`s.
* Ancillary data are in [].
* Complete requests proceed to `handleFrame` eagerly, where FDs can be tagged by `streamId`.

Easy cases:

```
  ; [fds for req1] req1<handleFrame> ;
  ; [fds for req1] req1 ; req1<handleFrame> ;
```

Handled by `readBufLen` telling us the FDs are NOT for `req1`:

```
  ; [fds for req2] req1<handleFrame> req2 ; req2<handleFrame> ;
  ; req1 ; [fds for req2] req1<handleFrame> req2 ; req2<handleFrame> ;
```

**Impossible** since we would've added dummy ancillary data to `req2`:

```
  ; [fds for req1] req1<handleFrame> req2 ;
```
