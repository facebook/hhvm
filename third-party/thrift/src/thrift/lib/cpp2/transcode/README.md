# Thrift multi-protocol transcoding

Transcode wire-encoded messages directly between serialization formats, driven by Thrift schemas, without hand-written per-service code and without materializing an intermediate object model.

## Why this exists

It powers gateways that expose Thrift services over other protocols (REST/JSON, gRPC/protobuf): a request arrives in one encoding and must reach the backend in another. Doing that per service by hand does not scale, and routing every message through a general object model is too slow for the hot path. This library turns a schema into a transcoder that rewrites the bytes in a single pass.

## What it does

Given a source protocol, a target protocol, and a Thrift schema, it produces a transcoder that reads one wire format and writes another across the full Thrift type system. Supported formats are Thrift Compact, Thrift Binary, Protobuf, and JSON. The same transcoder can run interpreted or JIT-compiled.

## Design principles

- Schema-driven: Codecs are derived from `ServiceDescriptor` & `TypeSystem`, i.e. runtime description of Thrift services & types.
- Single-pass without a DOM: wire-to-wire, for high-performance.
- Two interchangeable engines: a portable interpreter and an optional LLVM JIT, chosen per deployment. LLVM is never a mandatory dependency.
- Safe on untrusted input: Provides the same deserialization guarantees as the Thrift protocol codecs.
