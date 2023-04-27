# Server Throughput Benchmarks

These benchmarks are designed to run different load scenarios to
measure what the performance of ThriftServer will look like.

To maximize QPS throughput, run the client and the server on different machines.

## Setting up the Server

Read about the different flags in the server:

`./server --helpon=Server`

By default, the server will calculate the number of cores in the
machine and it will set the number of IOThreads and CPU threads
based on that. To manually change that, use the integer flags.

In general, the server should be ready to go as is.

## Setting up the Client

Read about the different flags in the client:

`./client --helpon=Client`

Using this Client, it is possible to specify the number of Clients
that the machine will create. This will create a Client on its
own thread (to maximize throughput).

### Async

To maximize throughput, by default, this benchmark performs asynchronous calls.
A max number of outstanding opertion is set to avoid having the Client
overflow the server. That is, setting 50 Clients with a
max outstanding operations of 100 means that at most, the Server will have
5000 outstanding operations.

### Weights

By default, it will be performing `noop`s. That is, no operations.
These are simple pings to the server. These are used to measure
the theoretical limit of QPS. However, it is possible to specify
different operations to be performed. By checking the flags on the
client side, all the operations are specified by a `--*_weight`
flag. To perform every call with just that operation, set the flag
to 1, i.e.: `--*_weight=1`.

If a more varied scenario is required, it is possible to specify more
operations with different weights each. This means that every client
will at random select one of the operations with a distribution based
on the weights that were given to them. That is, specifying:
`--noop_weight=9 --sum_weight=1` will perform 90% of noops and 10% of weight.

### Exceptions

The weight operations are specially useful when testing a real production
environment which will have exceptions thrown every now and then. By using
exception operations, it will guarantee that an exception will be thrown.
Thus, it is possible to have the following scenario:
`--weight_noop=999 --weight_noop_ex=1`. That is, out of every 1000 calls,
it is guaranteed to have at least one exception thrown.

### Transports

Thrift is a transport agnostic RPC Framework. While header transport has
been used by for most of Thrift's lifetime, it is now possible to select
different transports. In this case, it is possible to do that by specifying
the transport flag. We currently support http2, and reactive sockets.

`--transport="header" (Default)`

`--transport="http2"`

`--transport="rocket"`

## Reading the metrics

On both on the client and the server side, the output will look like the following:

`./client --host="IP" --transport="header" --async --num_clients=100 --noop_weight=1 --sum_weight=1`

```
| QPS: 1.575878e+06 | Max QPS: 1.575878e+06 | Total Queries: 5.590453e+06 | Operation: sum
| QPS: 1.578935e+06 | Max QPS: 1.578935e+06 | Total Queries: 5.595629e+06 | Operation: noop
...
| TOTAL QPS: 3.154813e+06
```

This means that the current total throughput is 3.15MQPS, plus, it
breaks down the QPS per operation type to get better insights on the
operations that the client/server is preforming.

## Timeout testing

In the timeout testing the aim is not to force the server to its limits
but to observe that server stays functional even though some of the tasks
gets cancelled because of timing out.

`./client --host="IP" --transport="header" --async --num_clients=100 --max_outstanding_ops=1 --timeout_weight=1`

For example, the `timeout` function sleeps the worker thread for `1` ms.
When the number of clients is 100, each client will sleep for `1` ms and
if the number of threads is equal to 40, it will cause some of the threads
to sleep for `2` ms or even `3` ms.
The result should be some of the tasks getting cancelled while some of them
get executed successfully.

## Stream testing

Compare Single RPC download/upload perf against Streaming RPC download/upload of data.

`./client --host="IP" --transport="rocket" --num_clients=1 --max_outstanding_ops=1 --download_weight=1 --upload_weight=1`
`./client --host="IP" --transport="rocket" --num_clients=1 --max_outstanding_ops=1 --stream_weight=1`
