# Thrift Java V2 Cutover Migration

This document records how call sites move from the legacy Java thrift client
runtime to the v2 manager-backed runtime.

The main design point is that most callers do not switch to new v2 classes
directly. The generated client code now depends on the neutral
`RpcClientSource` seam, and runtime selection happens underneath the existing
builders and factories.

## Runtime Selection

There are three supported ways to select the runtime:

1. Global JVM flag

```bash
-Dthrift.client.runtime=v2
```

2. Per-client config for non-ServiceRouter clients

```java
new ThriftClientConfig().setClientRuntimeMode(ClientRuntimeMode.V2)
```

3. Per-factory config for ServiceRouter clients

```java
new ServiceRouterProxyClientConfig().setClientRuntimeMode(ClientRuntimeMode.V2)
```

If no runtime is specified, the default remains `legacy`.

## Precedence

### Non-ServiceRouter

1. `ThriftClientConfig.clientRuntimeMode`
2. `-Dthrift.client.runtime`
3. `legacy`

### ServiceRouter

1. `ServiceRouterProxyClientConfig.clientRuntimeMode`
2. `-Dthrift.client.runtime`
3. `legacy`

This allows a narrow canary on one client or one factory before changing the
whole JVM.

## What Changed in Generated Clients

Generated reactive, async, and blocking clients now acquire transports through
`RpcClientSource`.

That gives two compatible backing implementations:

- `LegacyRpcClientSource`
  - wraps the historical `Mono<RpcClient>`
  - `dispose()` is a no-op to preserve legacy behavior
- `BindingRpcClientSource`
  - wraps the v2 manager/binding stack
  - `dispose()` obeys v2 ownership semantics

Generated constructors that accept `Mono<? extends RpcClient>` still exist.
They are now a compatibility shim that routes through `ClientRuntimeSelector`.

## Non-ServiceRouter Migration

### Common Case: Build Through `RpcClientFactory`

Most callers already do this:

```java
ThriftClientConfig config = new ThriftClientConfig();

RpcClientFactory factory =
    RpcClientFactory.builder().setThriftClientConfig(config).build();

MyService client = MyService.clientBuilder().build(factory, address);
```

To cut that caller over, only set the runtime:

```java
ThriftClientConfig config =
    new ThriftClientConfig().setClientRuntimeMode(ClientRuntimeMode.V2);
```

No other call-site change is required.

What happens underneath:

- `RpcClientFactory.Builder.build()` resolves the runtime
- `legacy` returns the existing mono-based factory path
- `v2` returns `RpcClientFactoryV2`
- generated clients still call the same `clientBuilder().build(factory, address)`

### Direct `Mono<RpcClient>` Call Sites

Some older call sites construct generated reactive clients directly from a
`Mono<RpcClient>`.

That path still works:

```java
new MyServiceReactiveClient(protocolId, rpcClientMono)
```

Behavior:

- if the global runtime is `legacy`, the generated client gets a
  `LegacyRpcClientSource`
- if the global runtime is `v2`, the generated client gets a
  `BindingRpcClientSource` via `MonoBackedRpcClientManager`

If a raw-mono call site wants explicit per-call-site control instead of the
global flag, it can opt into the neutral seam directly:

```java
RpcClientSource source =
    ClientRuntimeSelector.createSource(rpcClientMono, ClientRuntimeMode.V2);

new MyServiceReactiveClient(protocolId, source)
```

Preferred migration direction for long-term clarity:

- prefer `ClientBuilder.build(factory, address)` over direct mono construction
- use the explicit `RpcClientSource` constructor only when a call site truly
  owns a custom acquisition path

## ServiceRouter Migration

ServiceRouter callers continue to use:

```java
ServiceRouterProxyClientFactory.builder() ... build()
```

and then:

```java
factory.getClientDirect(request)
```

To cut over one ServiceRouter factory:

```java
ServiceRouterProxyClientConfig config =
    new ServiceRouterProxyClientConfig()
        .setClientRuntimeMode(ClientRuntimeMode.V2);

ServiceRouterProxyClientFactory factory =
    ServiceRouterProxyClientFactory.builder().config(config).build();
```

No manager classes are exposed to customers.

What happens underneath:

- ServiceRouter still chooses bindings vs sidecar the same way it did before
- the selected implementation resolves the runtime mode
- `legacy` injects a `LegacyRpcClientSource`
- `v2` injects a manager-backed `BindingRpcClientSource`

Important lifecycle result:

- shared ServiceRouter clients use borrowed bindings in v2, so closing one typed
  client handle does not tear down shared process-level SR transports
- owned/sticky connections use owned bindings, so closing the client shuts down
  the underlying manager

## Customer-Facing Impact

### What Customers Do Not Need To Change

- generated thrift service interfaces
- `clientBuilder()` usage
- request/response code
- blocking vs async vs reactive wrappers
- ServiceRouter request construction

### What Customers May Change

- add `setClientRuntimeMode(ClientRuntimeMode.V2)` to a config object
- set `-Dthrift.client.runtime=v2` for a process-wide canary

## Recommended Rollout Plan

1. Canary one client or one ServiceRouter factory with the per-config runtime
   field.
2. Expand to a process-wide canary with `-Dthrift.client.runtime=v2`.
3. Leave call sites unchanged while collecting production confidence.
4. Flip the default only after the legacy path is no longer needed.

## Summary

Call-site migration is intentionally small:

- Non-SR factory callers: set runtime on `ThriftClientConfig`
- SR callers: set runtime on `ServiceRouterProxyClientConfig`
- Raw mono callers: keep existing code and use the global flag, or explicitly
  pass `RpcClientSource` for per-call-site control

If a caller does nothing, it stays on `legacy`.
