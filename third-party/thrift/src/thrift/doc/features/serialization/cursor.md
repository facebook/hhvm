# Cursor-based Serialization (CurSe)

:::note
This is an advanced, restrictive, C++-only feature for services that need to shave small CPU wins from Thrift serialization. If you are asking yourself whether this is right for your service, the answer is probably no.
:::

CurSe is an alternate C++ serialization API for Thrift that is designed to minimize the serialization-related CPU overhead of using Thrift for RPC. Target services would previously have all of their functions return and accept `binary` which they would use handcrafted custom serialization to interact with. Using this Thrift-provided API allows the data to be schematized and provides partial compatibility with Thrift serialization in other languages, eliminating the two biggest problems with custom serialization.

## Restrictions

Where regular Thrift codegen is optimized for usability first and performance second, CurSe makes the opposite tradeoff. This involves the introduction of the following restrictions:

- Only Binary protocol is supported. Clients must be configured properly to interact with opted-in methods. Requests using the default Compact protocol will be completed with a `TApplicationException`.
- Data produced by normal Thrift serialization requires fields to be sorted in field id order in the IDL file, or the struct to be annotated with `@thrift.SerializeInFieldIdOrder`, in order to be compatible with CurSe.
- Fields can only be read / written in order of increasing field id in the struct. (Skipping fields is permitted).
- All field ids must be positive and not equal to `INT16_MAX`. Implicit field ids are not supported.
- Structs/unions opted in to CurSe cannot be used as fields in other structs/unions or as container elements.
- Methods taking a Curse-enabled struct/union as a parameter may only take one parameter.
- Reading and writing cannot be interleaved, so structs are effectively immutable during reading.
- The user is responsible for keeping the `CursorSerializationWrapper` alive until all reading/writing is complete.

These restrictions are all fundamental to the performance wins, and if any of them do not work for a service then that service is not a candidate for using CurSe. Most services will not be.

## How to enable

Add `cpp_include "thrift/lib/cpp2/protocol/CursorBasedSerializer.h"` and `include "thrift/annotation/cpp.thrift"` to the top of the Thrift file and annotate individual structs and unions with `@cpp.UseCursorSerialization`. Doing so means all uses of the type name in an IDL file will be opted into CurSe. To keep the opted-out struct available one can instead apply the annotation to a typedef of the struct/union.

Documentation for the C++ APIs including examples is inlined in [CursorBasedSerializer.h](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/protocol/CursorBasedSerializer.h).

### Migration for existing methods

There are two strategies available for enabling CurSe for methods that are receiving traffic. The first one is the most foolproof.

#### Add new methods

Before:
```thrift
struct Foo {}
service FooService {
    Foo getFoo();
}
```

After:
```thrift
cpp_include "thrift/lib/cpp2/protocol/CursorBasedSerializer.h"
include "thrift/annotation/cpp.thrift"

struct Foo {}
@cpp.UseCursorSerialization
typedef Foo FooWrapper

service FooService {
    Foo getFoo();
    FooWrapper getFooV2();
}
```

This allows clients to migrate independently to the new method after assuring they are configuring the protocol correctly (and they have been built with any field order changes that were necessary).

The implementation of `getFooV2` can fall back to the implementation of `getFoo` by calling the `deserialize()` method on the `CursorSerializationWrapper`. (The reverse is also possible using the converting constructor of `CursorSerializationWrapper`, but this incurs an extra serialization round-trip).

#### Update methods in-place

In this approach the annotation is applied to the existing structs used as parameters. All clients (in all languages) must first be sending all of their traffic using Binary protocol and use serialization code generated with the correct field order. All C++ clients and handler implementations must be updated to convert between `CursorSerializationWrapper` and the regular generated structs in the same diff that applies the annotation.

Before:
```cpp
Request req = ...;
Response resp = co_await client->co_foo(req);
use(resp);
```

After:
```cpp
Request req = ...;
Response resp = (co_await client->co_foo(req)).deserialize();
use(resp);
```

### Configuring Binary protocol

<FbInternalOnly>
Clients using ServiceRouter (recommended) can be migrated wholesale by the service owner by editing the service config. The protocol can be changed for the whole service, or just for methods that will use CurSe by [defining a config scope with a method resolution rule](https://www.internalfb.com/intern/wiki/ServiceRouter/User_Guide/How_to_Configure/Config_Scopes/#define-resolution-rules).
</FbInternalOnly>

Raw Thrift clients must be configured at each creation site. Note that for `PooledRequestChannel` the protocol must be specified both as an argument to `newChannel` and to the underlying channel in the factory function.
