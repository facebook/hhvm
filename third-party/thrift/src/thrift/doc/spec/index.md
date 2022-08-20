---
sidebar_position: 2
---

# Specification

Thrift supports clients/servers/readers/writers/etc, written in different languages, using different libraries, often developed and maintained by different teams. A basic example is a Java Thrift client invoking functions implemented by a C++ Thrift server. In order to have predictable and reliable behavior in such a heterogeneous environment, all Thrift libraries must support the same protocols and semantics.

These documents describe the specification that ensures consistent and predictable behavior across Thrift implementations:

- [Interface Definition Language (IDL)](idl.md).
- [Definitions](definition/index.md)
- [Protocols](protocol/index.md)
- [Beta Features](beta/index.md)
- [Experimental Features](experimental/index.md)
