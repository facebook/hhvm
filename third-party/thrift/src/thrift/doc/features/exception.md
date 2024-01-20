# Errors and Exceptions

:::note
This document covers aspirational behavior of existing Thrift implementations.
Thrift does not currently support all cases below
:::

How error classifications, functions qualifiers, and exceptions work together.

## Exceptions

A **declared** exception is an exception struct defined in IDL. Interfaces can declare that they might throw a declared exception using the `throws` keyword. Each method can specify a set of declared exceptions. Please refer to the [schema](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift) for more detail and syntax.

An **undeclared** exception is thrown by Thrift methods that are wrapped into a generic [TApplicationException](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp/TApplicationException.h) or [TTransportException](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp/transport/TTransportException.h) and passed to the client. It is similar to throwing generic exceptions (e.g. `Exception` or `std::exception`) and is generally discouraged since it is too general to carry sufficient information. It is highly encouraged to avoid as much as possible throwing undeclared exceptions.

### Exception Message

A custom (human-readable) exception message may be specified in a field, by
annotating it with
[`@thrift.ExceptionMessage`](/idl/annotations.md#thrift-annotations).

## Error Classification
### Kind
Thrift Error can be classified into three categories: **Transient**, **Stateful**, and **Permanent**.

|Choices|Source of Truth|Meaning|Retriable?|
|---|---|---|---|
|Transient|Exception definition in the server IDL  |Might succeed if retried immediately.  |Maybe  |
|Stateful |Exception definition in the server IDL  |State must be changed before any chance of success.  |No |
|Permanent  |Exception definition in the server IDL  |Can never succeed. |No |

Only a **transient** error is a candidate for internal retries. A **stateful** error requires application specific actions to be taken before a retry can succeed, for example, granting access to a server resource. A **permanent** error fundamentally can never succeed. An error kind can be set using the keyword `transient`, `stateful`, and `permanent`. Please refer to the [schema](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift) for more detail and syntax.

### Fault Attribution

Attribution of fault for errors is classified by which party is at fault:

|Choices|Source of Truth|Meaning|
|---|---|---|
|Unspecified|Exception definition in the server IDL |The default. The server is assumed to be at fault for retry purposes.  |
|Server|Exception definition in the server IDL |The server is at fault.
|Client|Exception definition in the server IDL |The client is at fault.

This is also typically statically associated with an Exception/ErrorCode, For example, it is built in to the HTTP error code space, where 4xx is a client error and 5xx is a server error. So, it will only be configurable in the IDL, and the server is used as the source of truth. An error fault attribution can be set using the keyword `server` and `client`. Please refer to the [schema](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift) for more detail and syntax.

### Error Safety

|Choices|Source of Truth|Meaning|
|---|---|---|
|Unspecified|Exception definition in the server IDL  |The default, error is assumed to not be safe.|
|Safe|Exception definition in the server IDL  | Failed RPC is known to have no side-effects.|

A safe error guarantees that there are no significant side effects from having tried to process the request. For example, if a pre-condition check fails before the request was processed, that error is safe. It is always ok to retry a safe error. State and permanent errors are often statically associated safe. Transient errors are rarely safe. An error can be marked safe using the keyword `safe`. Please refer to the [schema](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift) for more detail and syntax.

### Example

```thrift
// Indicates the underlying machine holding the data could not be reached.
//
// For this error, the service is at fault, it is safe to retry, and a retry
// might succeed if retried immediately.
safe transient server exception DataUnavailableError {...}

// Indicates the client is sending data type that is not supported by the server.
//
// This error is safe to retry, but it will never succeed, and it originates from the
// client.
safe permanent client exception UnSupportedType {...}

// None of the qualifiers are required and may be ommitted, in which case the default for that category will be used.
// Assumed to be not safe.
permanent client exception UnSupportedType {...}
```

## RPC Idempotency

**RPC idempotency** determines the retryability of the RPC method using reserved function qualifier keywords `readonly` and `idempotent`. A *readonly* method has no meaningful side effects, so it can be called any time. A *idempotent* method can be immediately retried after a transient failure. Please refer to the [schema](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift) and the [IDL reference](/idl/index.md#services) for more detail and syntax.

### Example

```thrift
service MyService {
  // It does not change the service, so it is always safe to rety (even after a success).
  readonly Data getData(...);
  // Safe to rety immeidately after a transient failure.
  idempotent void deleteData(...);
  // Not safe to retry unless the error is 'safe'.
  void updateData(...);
}
```

## Retryability

RPC Retry is enabled by default. Based on the error classification and RPC idempotency, the retry policy can be defined as the following:

```
Retryable = Error.Transient &&
    (RPC.Idempotent || RPC.Readonly || Error.Safe)
```

Note that, if the error kind is unspecified, it is assumed to be transient, and if the error is not known to be safe, it is assumed to not be safe. Thus the ‘default’ behavior is to only retry if the RPC is idempotent.

Unfortunately, transient errors, which are the only ones worth retrying, are rarely known to always be safe. However, most transient errors are produced by Thrift itself and known internally to be safe.
