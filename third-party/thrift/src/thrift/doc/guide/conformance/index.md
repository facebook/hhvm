---
sidebar_position: 2
---

# Conformance

## Overview

Thrift allows client and servers to be written in different programming languages. In order to have predictable and reliable behavior in such a heterogeneous environment, all Thrift implementations must support the same semantics when crossing a serialization boundary. This boundary includes serializing and deserializing a Thrift object, as well as invoking a Thrift RPC.

Thrift conformance consist of a framework which includes a thrift test client, a test server and a test runner. It allows plugging a thrift client and/or a server for the target language to the conformance test environment. It validates the target language client and/or server and produce a conformant test result.

### Levels

There are three different levels of conformance we test for:

1. **Consistency** - This category of conformance describes the semantics when a reader and writer use the same Thrift specification. This includes semantics regarding representability, equivalence, equality, comparison, and hashing.
2. **Compatibility** - This category of conformance describes the semantics when the readers and writers interpret data using different Thrift specifications. This is a common occurrence when upgrading a Thrift specification in a non-atomic fashion or reading previously stored serialized Thrift values. For example, when rolling out new clients/servers in a multi-machine environment or reading serialized values from disk or a database.
3. **Tolerance** - This category of conformance describes the expected behavior when a reader encounters malformed data or misbehaving peers.

### Categories

There are three different categories of conformance tests:

- **[Data](testsuite/data.md)** - Tests focus on serializing and deserializing the data.
- **[Client](testsuite/client-rpc.md)** - This category of conformance describes the client behavior when making a RPC call.
- **[Server](testsuite/server-rpc.md)** - This category of conformance describes the server behavior when making a RPC call.
Client/Server test focus on invoking remote procedure calls (RPCs) with request-response, streaming, sink and interactions, while data tests cover the (de)serialization of Thrift values in any context (RPC, disk, db, etc).

### Conformance tests

- [Conformance Quickstart Guide](fb/quickstart.md)
- [Test Suites](testsuite/index.md)
