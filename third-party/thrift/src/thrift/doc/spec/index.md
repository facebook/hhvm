---
description: Learn about Thrift Conformance Specification!
---

# Specification

Thrift supports clients/servers/readers/writers/etc, written in different languages, using different libraries, often developed and maintained by different teams. A basic example is a Java Thrift client invoking functions implemented by a C++ Thrift server. In order to have predictable and reliable behavior in such a heterogeneous environment, all Thrift libraries must support the same protocols and semantics.

These documents describe the specification that ensures consistent and predictable behavior across Thrift implementations:

import DocCardList from '@theme/DocCardList';

<DocCardList />

# Enforcing the spec

To ensure this consistent and predictable behavior, the spec is enforced through a set of [Conformance Tests](../contributions/conformance).
