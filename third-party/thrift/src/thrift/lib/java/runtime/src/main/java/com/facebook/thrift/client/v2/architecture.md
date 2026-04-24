# V2 Client Runtime Architecture

This document describes the internal architecture of the v2 Java Thrift client
runtime. For the customer-facing migration guide, see [migration.md](migration.md).

## Design Goal

Separate **transport creation** (how connections are established and decorated)
from **connection lifecycle** (who owns connections, when they are reused, and
who may dispose them). The legacy runtime conflated both inside
`Mono<RpcClient>`, making ownership implicit. The v2 runtime makes it explicit.

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
│                   GENERATED WRAPPERS (unchanged)                     │
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
│                   GENERATED ReactiveClient (simplified)              │
│                   e.g. MyServiceReactiveClient                       │
│                                                                      │
│   Field:  RpcClientSource _clientSource                              │
│                                                                      │
│   dispose()    { _clientSource.dispose(); }                          │
│   isDisposed() { return _clientSource.isDisposed(); }                │
│                                                                      │
│   doStuffWrapper(request, rpcOptions) {                              │
│     return _clientSource.acquire()                                   │
│       .flatMap(rpc -> /* serialize, send, deserialize */);           │
│   }                                                                  │
│                                                                      │
│   Translates typed method calls into protocol-level payloads.        │
│   Has NO connection tracking, NO wrapper identity sets.              │
│   Delegates all lifecycle to the source.                             │
└────────────────────────────────┬─────────────────────────────────────┘
                                 │ acquire() / dispose()
                                 ▼
┌──────────────────────────────────────────────────────────────────────┐
│                     RpcClientSource (neutral seam)                   │
│                                                                      │
│   interface RpcClientSource extends Disposable {                     │
│     Mono<RpcClient> acquire();                                       │
│   }                                                                  │
│                                                                      │
│   Two implementations:                                               │
│                                                                      │
│   LegacyRpcClientSource              BindingRpcClientSource          │
│   ├─ wraps Mono<RpcClient>           ├─ wraps RpcClientBinding       │
│   ├─ acquire() = mono                ├─ acquire() = binding.acquire()│
│   └─ dispose() = no-op              └─ dispose() = binding.dispose() │
│                                                                      │
│   Selected by ClientRuntimeSelector based on ClientRuntimeMode.      │
│   Generated code is identical for both—only the source differs.      │
└────────────────────────────────┬─────────────────────────────────────┘
                                 │ (v2 path only below)
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
│                                                                      │
│   ┌─────────────────────────────────────────────────────────────┐    │
│   │ MonoBackedRpcClientManager                                  │    │
│   │   compatibility bridge for existing Mono<RpcClient> sources │    │
│   │   tracks last-seen RpcClient for disposal                   │    │
│   │   used by legacy build(Mono) paths in v2 mode               │    │
│   └─────────────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────────────┘
                                 │ manager calls transportFactory
                                 ▼
┌──────────────────────────────────────────────────────────────────────┐
│                     TRANSPORT LAYER (RpcClientFactory)               │
│                     (stateless, per-connection decoration)           │
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

## Factory Hierarchy

Factories create managers. Managers own transports.

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

The standard non-SR factory chain built by `RpcClientFactoryV2.Builder`:

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

// Each typed client borrows the shared manager
MyService clientA = clientBuilder.buildBorrowed(sharedManager);
MyService clientB = clientBuilder.buildBorrowed(sharedManager);

clientA.close();  // → binding.dispose() → BORROWED → closed=true only
                  //   transport untouched, clientB still works

clientB.doStuff();  // works fine, shared connection is alive
```

## How-To Guide

### Create a standard non-SR client

```java
// The factory resolves to v2 when the runtime is set
ThriftClientConfig config = new ThriftClientConfig()
    .setClientRuntimeMode(ClientRuntimeMode.V2);

RpcClientFactory factory = RpcClientFactory.builder()
    .setThriftClientConfig(config)
    .build();

MyService client = MyService.clientBuilder()
    .build(factory, address);

// Use the client
MyResponse response = client.myMethod(request);

// Close when done — disposes the underlying transport cleanly
client.close();
```

### Create shared SR clients (borrowed)

```java
// Create ONE shared manager (lives for the process)
RpcClientManager sharedManager =
    new DecoratingRpcClientManager(
        managerFactory.createRpcClientManager(socketAddress),
        ExceptionMappingRpcClient::new);

// Create typed clients that borrow the shared manager
MyService client = MyService.clientBuilder()
    .build(ClientRuntimeSelector.createBorrowedSource(sharedManager));

// client.close() only closes the handle — the connection stays alive
client.close();

// Create more clients on the same shared manager
MyService anotherClient = MyService.clientBuilder()
    .build(ClientRuntimeSelector.createBorrowedSource(sharedManager));
```

### Create a client with explicit ownership

```java
// Direct manager construction
RpcClientManager manager = managerFactory.createRpcClientManager(address);

// OWNED — closing this client kills the manager and its transports
MyService ownedClient = MyService.clientBuilder()
    .build(ClientRuntimeSelector.createOwnedSource(manager));

// BORROWED — closing this client only closes the handle
MyService borrowedClient = MyService.clientBuilder()
    .build(ClientRuntimeSelector.createBorrowedSource(manager));
```

### Create a direct-to-tier pooled client

```java
// PooledRpcClientManagerFactory manages per-tier host pools
PooledRpcClientManagerFactory poolFactory = new PooledRpcClientManagerFactory(
    delegateFactory, hostSelectFunction, poolSize);

// Each tier gets one shared pool; clients borrow from it
RpcClientManager tierManager = poolFactory.createRpcClientManager(
    new TierSocketAddress("my-service"));

MyService client = MyService.clientBuilder()
    .build(ClientRuntimeSelector.createBorrowedSource(tierManager));

// Shut down all pools at process exit
poolFactory.dispose();
```

## Key Design Decisions

### Why RpcClientSource instead of RpcClientBinding in generated code?

`RpcClientSource` is a neutral interface that both legacy and v2 can implement.
Generated code depends on this seam, so the same generated `.class` files work
in both modes. The runtime selection happens in `ClientRuntimeSelector`, not in
codegen.

### Why is CachedRpcClientFactory excluded from the v2 transport factory?

`SingleRpcClientManager` does its own caching — it stores the live `rpcClient`
and returns `Mono.just(current)` on the hot path. Adding
`CachedRpcClientFactory` would be redundant.

### Why doesn't the manager layer include per-request timeout?

The transport factory chain includes `TimeoutRpcClientFactory`, which wraps the
`RpcClient` with per-request timeout behavior. This applies to every connection
the manager creates. The manager layer handles connection-level concerns
(reconnection, pooling, selection), not request-level concerns (timeout).

### Why is SimpleLoadBalancingRpcClientManager skipped for poolSize == 1?

Wrapping a single manager in a load-balancing container adds an indirection with
no benefit. The old legacy path wraps even with `poolSize >= 1` due to a
historical quirk.

## File Inventory

### v2/manager/ — Lifecycle management

| File | Purpose |
|------|---------|
| `RpcClientManager.java` | Core lifecycle interface |
| `RpcClientManagerFactory.java` | Factory interface |
| `ClientOwnership.java` | OWNED / BORROWED enum |
| `RpcClientBinding.java` | Ownership gate between client and manager |
| `AbstractRpcClientManager.java` | Shared close-state template |
| `SingleRpcClientManager.java` | One lazily-connected transport |
| `ReconnectingRpcClientManager.java` | Retry wrapper over Single |
| `SimpleLoadBalancingRpcClientManager.java` | Round-robin / sticky over N children |
| `PooledRpcClientManager.java` | Dynamic host pool with periodic refresh |
| `DecoratingRpcClientManager.java` | Stateless RpcClient decoration |
| `MonoBackedRpcClientManager.java` | Compatibility bridge for Mono sources |
| `*Factory.java` | Corresponding factories for each manager |

### v2/transport/ — V2 factory entrypoint

| File | Purpose |
|------|---------|
| `RpcClientFactoryV2.java` | Builds both legacy mono factory and manager factory |
| `BindingRpcClientSource.java` | V2 implementation of RpcClientSource |

### client/ — Neutral seam

| File | Purpose |
|------|---------|
| `RpcClientSource.java` | Interface: acquire() + Disposable |
| `LegacyRpcClientSource.java` | Legacy implementation (dispose = no-op) |
| `ClientRuntimeSelector.java` | Routes to legacy or v2 based on config |
| `ClientRuntimeMode.java` | LEGACY / V2 enum |
| `ClientBuilder.java` | Abstract builder, now uses RpcClientSource |
