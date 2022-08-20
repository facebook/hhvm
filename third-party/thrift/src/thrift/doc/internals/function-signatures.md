# Function signatures

Here is the table of function signatures for generated client functions:

## Legend

T = the type for the argument or return value

```
sync_input = apache::thrift2::SyncInputStream
sync_output = apache::thrift2::SyncOutputStream
async_input = apache::thrift2::AsyncInputStream
async_output = apache::thrift2:AsyncOutputStream

functor = std::function<void (::apache::thrift2::ClientReceiveState&&)>
future = folly::Future<T>
unit = folly::Unit
stream_singleton = apache::thrift2::StreamSingleton
```

std::function functions
================

*std::function functions are not generated for functions using streams*

## name prefix
(none), functor_

## return type

```
thrift return |    no stream parameters                with stream parameters
    type      |
---------------------------------------------------------------------------
void          |    void f(functor, ...)                    not available
              |
simple        |    void f(functor, ...)                    not available
              |
complex       |    void f(functor, ...)                    not available
              |
stream        |       not available                        not available
```

## parameter type

```
simple        |           T
              |
complex       |       const T&
              |
stream        |     not available
```

sync functions
================

## name prefix
sync_

## return type

```
thrift return |    no stream parameters                with stream parameters
    type      |
---------------------------------------------------------------------------
void          |       void f(...)                            void  f(...)
              |
simple        |         T f(...)                      stream_singleton<T> f(...)
              |
complex       |     void f(T&, ...)                   stream_singleton<T> f(...)
              |
stream        |   sync_input<T> f(...)                    sync_input<T> f(...)
```

## parameter type

```
simple        |         T
              |
complex       |      const T&
              |
stream        |   sync_output<T>&
```

future functions
================

## name prefix
future_

## return type

```
thrift return |    no stream parameters                with stream parameters
    type      |
---------------------------------------------------------------------------
void          |    future<unit> f(...)                  future<unit> f(...)
              |
simple        |     future<T> f(...)                      future<T> f(...)
              |
complex       |     future<T> f(...)                      future<T> f(...)
              |
stream        |future<unit> f(async_input<T>&, ...)
              |                            future<unit> f(async_input<T>&, ...)
```

## parameter type

```
simple        |           T
              |
complex       |        const T&
              |
stream        |     sync_output<T>&
```

async functions
================

## name prefix
(none), callback_

## return type

```
thrift return |    no stream parameters                with stream parameters
    type      |
---------------------------------------------------------------------------
void          |       void f(...)                         void  f(...)
              |
simple        |         T f(...)                    void f(async_input<T>&, ...)
              |
complex       |     void f(T&, ...)                 void f(async_input<T>&, ...)
              |
stream        | void f(async_input<T>&, ...)        void f(async_input<T>&, ...)
```

## parameter type

```
simple        |           T
              |
complex       |        const T&
              |
stream        |    async_output<T>&
```

server functions
================

## name prefix
(none)

## return type

```
thrift return |    no stream parameters                with stream parameters
    type      |
---------------------------------------------------------------------------
void          |       void f(...)                         void f(...)
              |
simple        |         T f(...)                   void f(async_output<T>&, ...)
              |
complex       |     void f(T&, ...)                void f(async_output<T>&, ...)
              |
stream        | void f(async_output<T>&, ...)      void f(async_output<T>&, ...)
```

## parameter type

```
simple        |           T
              |
complex       |    std::unique_ptr<T>
              |
stream        |     async_input<T>&
```
