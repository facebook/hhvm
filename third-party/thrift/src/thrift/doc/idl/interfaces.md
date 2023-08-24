---
state: draft
---

# Interfaces

Types that send and/or receive data using [interface protocols](/fb/server/interface/index.md).

## Service

A **service** is an interface for RPC defined in Thrift. Each service has a set of functions. Each **function** has a unique name and takes a list of arguments. A function can throw an exception if the server handler chooses to throw an exception or if there was an issue with the RPC itself. The list of arguments follows a similar rule to Thrift struct type with the exception of field qualifier. A function can be qualified with a **function qualifier** such as the following:

- `oneway`: the client does not expect response back from server.
- `idempotent`: safe to retry immediately after a transient failure.
- `readonly`: always safe to retry.

A service can extend other services, inheriting the set of functions included in the inheriting service using the reserved keyword `extends`. Please refer to the [schema](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift) for more detail and syntax.

## Request Response

A **request reponse** is a method that takes a single payload for the request and waits for a single payload for the response.

## Request No Response

A **request no response** is a method that takes a single payload for the request, but it does not wait for the response. It is enabled with `oneway` function qualifier keyword.

## Stream

A **stream** is a communication abstraction between a client and server, where a server acts as the producer and the client acts as the consumer. It allows the flow of ordered messages from the server to the client. All messages in the stream have same payload object type. It may initially return an initial response specified in the IDL. The client can choose to cancel the stream at any time. The server can terminate the stream by sending the exception. Please refer to the [schema](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift) and the [protocol spec](/fb/server/interface/index.md) for more information about streams.

## Sink

A **sink** is similar to a stream, but the client acts as the producer and the server acts as the consumer. It allows the flow of ordered messages from the client to the server. It may initially return an initial response specified in the IDL, and it always returns a final response. The client will wait for a final response back from the server marking the completion of the sink. The client can terminate the sink by sending an exception to the server. The server can also terminate the sink by sending an exception while consuming payloads. The exception acts as the termination of the sink. Therefore, the client will not wait for the final response back from the server when it receives an exception. Please refer to the [schema](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift) and the [protocol spec](/fb/server/interface/index.md) for more information about sinks.
