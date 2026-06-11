# Java Thrift Client Runtime Architecture

This document describes the internal architecture of the Java Thrift client
runtime.

> Historical note: this directory is named `v2/` because the manager-backed
> runtime was the second iteration. The original `Mono<RpcClient>`-based
> runtime ("v1") was removed once the manager-backed runtime became the
> default. There is no longer a "legacy vs v2" choice — this is the runtime.

## Design Goal

Separate **transport creation** (how connections are established and decorated)
from **connection lifecycle** (who owns connections, when they are reused, and
who may dispose them). Transport factories are stateless and per-connection;
managers own and recycle live transports; bindings gate per-typed-client
ownership.

## Layer Diagram

```
┌──────────────────────────────────────────────────────────────────────┐
│                           USER CODE                                  │
│                                                                      │
│   MyService client = MyService.clientBuilder()                       │
│       .build(factory, address);                                      │
│   client.doStuff();                                                  │
│   client.close();                                                    │
└────────────────────────────────┬─────────────────────────────────────┘
                                 │ close() / dispose()
                                 ▼
┌──────────────────────────────────────────────────────────────────────┐
│                   GENERATED WRAPPERS                                 │
│             ReactiveBlockingWrapper / ReactiveAsyncWrapper           │
│                                                                      │
│   close() { _delegate.dispose(); }                                   │
│   doStuff() { _delegate.doStuffWrapper(...).block(); }               │
│                                                                      │
│   Pure calling-convention adaptation. No lifecycle logic.            │
└────────────────────────────────┬─────────────────────────────────────┘
                                 │ dispose()
                                 ▼
┌──────────────────────────────────────────────────────────────────────┐
│                   GENERATED ReactiveClient                           │
│                   e.g. MyServiceReactiveClient                       │
│                                                                      │
│   Field:  RpcClientBinding _binding                                  │
│                                                                      │
│   dispose()    { _binding.dispose(); }                               │
│   isDisposed() { return _binding.isDisposed(); }                     │
│                                                                      │
│   doStuffWrapper(request, rpcOptions) {                              │
│     return _binding.acquire()                                        │
│       .flatMap(rpc -> /* serialize, send, deserialize */);           │
│   }                                                                  │
│                                                                      │
│   Translates typed method calls into protocol-level payloads.        │
│   Has NO connection tracking. Delegates all lifecycle to binding.    │
└────────────────────────────────┬─────────────────────────────────────┘
                                 │ acquire() / dispose()
                                 ▼
┌──────────────────────────────────────────────────────────────────────┐
│                     RpcClientBinding                                 │
│                     (the ownership decision point)                   │
│                                                                      │
│   Fields:                                                            │
│     RpcClientManager manager                                         │
│     ClientOwnership ownership    // OWNED or BORROWED                │
│     AtomicBoolean closed                                             │
│                                                                      │
│   acquire():                                                         │
│     if (closed) → Mono.error("client is closed")                     │
│     else        → manager.acquire()                                  │
│                                                                      │
│   dispose():                                                         │
│     closed = true                                                    │
│     if (OWNED) → manager.dispose()    // kills transport             │
│     if (BORROWED) → nothing           // transport lives             │
│                                                                      │
│   This is the ONLY class that knows about ownership.                 │
│   Everything above calls dispose() without knowing the policy.       │
│   Everything below doesn't know it's being shared.                   │
└────────────┬───────────────────────────────────────┬─────────────────┘
             │ OWNED: manager.dispose()              │ BORROWED: no-op
             ▼                                       │
┌──────────────────────────────────────────────────────────────────────┐
│                     RpcClientManager                                 │
│                     (lifecycle owner for connections)                │
│                                                                      │
│   interface RpcClientManager extends Disposable {                    │
│     Mono<RpcClient> acquire();                                       │
│     Mono<Void> onClose();                                            │
│   }                                                                  │
│                                                                      │
│   Implementations:                                                   │
│                                                                      │
│   ┌─────────────────────────────────────────────────────────────┐    │
│   │ SimpleLoadBalancingRpcClientManager                         │    │
│   │   N child managers, round-robin or sticky-hash selection    │    │
│   │   dispose() cascades to all N children                      │    │
│   │                                                             │    │
│   │   ┌──────────────────────────────────────────────────┐      │    │
│   │   │ ReconnectingRpcClientManager                     │      │    │
│   │   │   wraps SingleRpcClientManager                   │      │    │
│   │   │   retries acquire() with exponential backoff     │      │    │
│   │   │   stops retrying when disposed                   │      │    │
│   │   │                                                  │      │    │
│   │   │   ┌───────────────────────────────────────┐      │      │    │
│   │   │   │ SingleRpcClientManager                │      │      │    │
│   │   │   │   owns ONE live RpcClient at a time   │      │      │    │
│   │   │   │   subscribes to onClose() for         │      │      │    │
│   │   │   │     auto-invalidation                 │      │      │    │
│   │   │   │   lazily creates replacement on       │      │      │    │
│   │   │   │     next acquire()                    │      │      │    │
│   │   │   │   dispose() kills the live client     │      │      │    │
│   │   │   └───────────────────────────────────────┘      │      │    │
│   │   └──────────────────────────────────────────────────┘      │    │
│   └─────────────────────────────────────────────────────────────┘    │
│                                                                      │
│   ┌─────────────────────────────────────────────────────────────┐    │
│   │ PooledRpcClientManager                                      │    │
│   │   per-address child managers from a periodically            │    │
│   │   refreshed host list (1-min poll)                          │    │
│   │   reuses managers for unchanged addresses                   │    │
│   │   disposes managers for removed addresses                   │    │
│   │   dispose() stops polling + kills all children              │    │
│   └─────────────────────────────────────────────────────────────┘    │
│                                                                      │
│   ┌─────────────────────────────────────────────────────────────┐    │
│   │ DecoratingRpcClientManager                                  │    │
│   │   .map(decorator) on each acquire()                         │    │
│   │   lifecycle delegates to inner manager                      │    │
│   │   used for ExceptionMappingRpcClient in SR                  │    │
│   └─────────────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────────────┘
                                 │ manager calls transport factory
                                 ▼
┌──────────────────────────────────────────────────────────────────────┐
│                     TRANSPORT LAYER                                  │
│                     (RpcClientTransportFactory)                      │
│                     stateless, per-connection decoration             │
│                                                                      │
│   @FunctionalInterface                                               │
│   interface RpcClientTransportFactory {                              │
│     Mono<RpcClient> createRpcClient(SocketAddress address);          │
│   }                                                                  │
│                                                                      │
│   RSocketRpcClientFactory / LegacyRpcClientFactory                   │
│     → InstrumentedRpcClientFactory (.map)                            │
│     → TokenPassingRpcClientFactory (.map)                            │
│     → EventHandlerRpcClientFactory (.map)                            │
│     → TimeoutRpcClientFactory (.map)                                 │
│                                                                      │
│   "Given an address, create ONE connection, wrap it with             │
│    stats/tokens/events/timeout, and return it."                      │
│                                                                      │
│   No lifecycle. No caching. No reconnection. Stateless.              │
│   The manager layer calls this when it needs a new connection.       │
└──────────────────────────────────────────────────────────────────────┘
                                 │ creates
                                 ▼
┌──────────────────────────────────────────────────────────────────────┐
│                     RpcClient (raw transport)                        │
│                                                                      │
│   interface RpcClient extends Disposable {                           │
│     Mono<ClientResponsePayload<T>>                                   │
│       singleRequestSingleResponse(payload, options);                 │
│     Mono<Void> onClose();                                            │
│   }                                                                  │
│                                                                      │
│   dispose() = send ERROR frame + close TCP channel.                  │
│   This is the physical resource that must be cleaned up.             │
└──────────────────────────────────────────────────────────────────────┘
```

## Two Factory Interfaces

The runtime has two distinct factory interfaces at different layers:

| Interface                     | Layer            | Produces             | Caching / lifecycle |
|------------------------------ |----------------- |--------------------- |---------------------|
| `RpcClientFactory`            | binding (public) | `RpcClientBinding`   | yes — wraps a manager |
| `RpcClientTransportFactory`   | transport (raw)  | `Mono<RpcClient>`    | no — stateless        |

`RpcClientFactory` is the public entrypoint that typed-client builders consume.
`RpcClientTransportFactory` is the lower-level seam that the manager layer
calls when it needs a fresh connection. The two interfaces never collapse:
their callers, return types, and lifecycle expectations are different.

## Factory Hierarchy

```
RpcClientManagerFactory                         (functional interface)
  │
  ├── SingleRpcClientManagerFactory             → SingleRpcClientManager
  │     one lazily-connected transport per address
  │
  ├── ReconnectingRpcClientManagerFactory       → ReconnectingRpcClientManager
  │     wraps Single with retry backoff
  │
  ├── SimpleLoadBalancingRpcClientManagerFactory → SimpleLoadBalancingRpcClientManager
  │     N child managers per address, round-robin
  │
  └── PooledRpcClientManagerFactory             → PooledRpcClientManager
        one pool per tier, periodic host refresh
```

The standard non-SR factory chain built by `RpcClientFactory.Builder`:

```
SimpleLoadBalancingRpcClientManagerFactory (if poolSize > 1)
  └── ReconnectingRpcClientManagerFactory (or SingleRpcClientManagerFactory)
        └── transport factory (RSocket/Legacy + stats + tokens + events + timeout)
```

## Ownership Model

The ownership decision is made at the point where a typed client is created,
not inside the generated code or the transport layer.

### OWNED — client controls the connection lifecycle

```java
// User creates a client and owns it
MyService client = MyService.clientBuilder().build(factory, address);
client.doStuff();
client.close();  // → binding.dispose() → OWNED → manager.dispose()
                 //   → SingleRpcClientManager.doDispose()
                 //   → rpcClient.dispose() → ERROR frame → channel closed
```

### BORROWED — client is a handle over a shared connection

```java
// SR sidecar creates one shared manager for the process lifetime
RpcClientManager sharedManager = managerFactory.createRpcClientManager(socketAddress);

// Each typed client borrows the shared manager via a BORROWED binding
RpcClientBinding bindingA = new RpcClientBinding(sharedManager, ClientOwnership.BORROWED);
RpcClientBinding bindingB = new RpcClientBinding(sharedManager, ClientOwnership.BORROWED);

MyService clientA = MyService.clientBuilder().build(bindingA);
MyService clientB = MyService.clientBuilder().build(bindingB);

clientA.close();  // → bindingA.dispose() → BORROWED → closed=true only
                  //   transport untouched, clientB still works

clientB.doStuff();  // works fine, shared connection is alive
```

## How-To Guide

### Create a standard non-SR client

```java
RpcClientFactory factory = RpcClientFactory.builder()
    .setThriftClientConfig(new ThriftClientConfig())
    .build();

MyService client = MyService.clientBuilder()
    .build(factory, address);

// Use the client
MyResponse response = client.myMethod(request);

// Close when done — disposes the underlying transport cleanly
client.close();
```

### Create a client over a raw transport factory

When you have a transport factory directly (e.g. a `LegacyRpcClientFactory`)
and don't need the full `RpcClientFactory.Builder` chain:

```java
RpcClientTransportFactory transport =
    new LegacyRpcClientFactory(new ThriftClientConfig().setDisableSSL(true));

MyService client = MyService.clientBuilder()
    .build(transport, address);  // wires a SingleRpcClientManager internally
```

### Create shared SR clients (borrowed)

```java
// Create ONE shared manager (lives for the process)
RpcClientManager sharedManager =
    new DecoratingRpcClientManager(
        managerFactory.createRpcClientManager(socketAddress),
        ExceptionMappingRpcClient::new);

// Create typed clients that borrow the shared manager
RpcClientBinding binding =
    new RpcClientBinding(sharedManager, ClientOwnership.BORROWED);
MyService client = MyService.clientBuilder().build(binding);

// client.close() only closes the handle — the connection stays alive
client.close();
```

### Create a direct-to-tier pooled client

```java
// PooledRpcClientManagerFactory manages per-tier host pools
PooledRpcClientManagerFactory poolFactory = new PooledRpcClientManagerFactory(
    delegateFactory, hostSelectFunction, poolSize);

// Each tier gets one shared pool; clients borrow from it
RpcClientManager tierManager = poolFactory.createRpcClientManager(
    new TierSocketAddress("my-service"));

RpcClientBinding binding =
    new RpcClientBinding(tierManager, ClientOwnership.BORROWED);
MyService client = MyService.clientBuilder().build(binding);

// Shut down all pools at process exit
poolFactory.dispose();
```

## Key Design Decisions

### Why a separate `RpcClientTransportFactory` interface?

`RpcClientFactory` produces fully-configured bindings (with managers, pools,
reconnection). `RpcClientTransportFactory` produces single raw connections
with optional per-request decoration. They live at different layers and have
different lifecycle expectations. A single interface obscured this and made
the manager layer's contract ambiguous.

### Why is per-address caching not in the transport factory?

`SingleRpcClientManager` does its own caching — it stores the live `rpcClient`
and returns `Mono.just(current)` on the hot path. A separate caching layer in
the transport chain would be redundant.

### Why doesn't the manager layer include per-request timeout?

The transport factory chain includes `TimeoutRpcClientFactory`, which wraps the
`RpcClient` with per-request timeout behavior. This applies to every connection
the manager creates. The manager layer handles connection-level concerns
(reconnection, pooling, selection), not request-level concerns (timeout).

### Why is `SimpleLoadBalancingRpcClientManager` skipped for `poolSize == 1`?

Wrapping a single manager in a load-balancing container adds an indirection
with no benefit.

## File Inventory

### `client/` — Public API + transport factories

| File | Purpose |
|------|---------|
| `RpcClientFactory.java` | Public binding-factory class with `Builder` |
| `RpcClientTransportFactory.java` | `@FunctionalInterface` for raw-transport tier |
| `ClientBuilder.java` | Abstract typed-client builder |
| `RpcClient.java` | Raw transport contract |
| `DelegatingRpcClientFactory.java` | Base class for transport decorators |
| `InstrumentedRpcClientFactory.java` | Stats decoration |
| `TokenPassingRpcClientFactory.java` | Header-token decoration |
| `EventHandlerRpcClientFactory.java` | Event-handler decoration |
| `TimeoutRpcClientFactory.java` | Per-request timeout decoration |

### `v2/manager/` — Lifecycle management

| File | Purpose |
|------|---------|
| `RpcClientManager.java` | Core lifecycle interface |
| `RpcClientManagerFactory.java` | Manager-factory interface |
| `ClientOwnership.java` | OWNED / BORROWED enum |
| `RpcClientBinding.java` | Ownership gate between client and manager |
| `AbstractRpcClientManager.java` | Shared close-state template |
| `SingleRpcClientManager.java` | One lazily-connected transport |
| `ReconnectingRpcClientManager.java` | Retry wrapper over Single |
| `SimpleLoadBalancingRpcClientManager.java` | Round-robin / sticky over N children |
| `PooledRpcClientManager.java` | Dynamic host pool with periodic refresh |
| `DecoratingRpcClientManager.java` | Stateless `RpcClient` decoration |
| `*Factory.java` | Corresponding factories for each manager |
