# Fast Thrift Benchmark Suite

A performance benchmarking framework for measuring Fast Thrift throughput and latency characteristics under various payload sizes and concurrency levels.

## Overview

This benchmark suite measures the raw performance of the Fast Thrift channel pipeline by running echo-style request/response workloads. It supports concurrent client connections to measure true QPS (queries per second) rather than just round-trip latency.

### Architecture

```text
┌─────────────────────────────────────────────────────────────────┐
│                        BenchmarkClient                          │
│  ┌─────────────┐  ┌─────────────┐       ┌─────────────┐        │
│  │ClientThread │  │ClientThread │  ...  │ClientThread │        │
│  │  (evb 0)    │  │  (evb 1)    │       │  (evb N)    │        │
│  │  ┌───────┐  │  │  ┌───────┐  │       │  ┌───────┐  │        │
│  │  │Socket │  │  │  │Socket │  │       │  │Socket │  │        │
│  └──┴───────┴──┘  └──┴───────┴──┘       └──┴───────┴──┘        │
└────────┬──────────────────┬────────────────────┬────────────────┘
         │                  │                    │
         └──────────────────┼────────────────────┘
                            │ TCP
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                       BenchmarkServer                           │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                  ConnectionManager                        │  │
│  │  ┌─────────────────────────────────────────────────────┐ │  │
│  │  │            IOThreadPoolExecutor                     │ │  │
│  │  │  ┌─────────┐  ┌─────────┐       ┌─────────┐        │ │  │
│  │  │  │ IO Thd 0│  │ IO Thd 1│  ...  │ IO Thd M│        │ │  │
│  │  └──┴─────────┴──┴─────────┴───────┴─────────┴────────┘ │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Components

| Component | Description |
|-----------|-------------|
| `BenchmarkServer` | Echo server that accepts connections and echoes back payloads |
| `BenchmarkClient` | Client driver that runs benchmark tests and collects statistics |
| `ClientRunner` | Manages multiple client threads with dedicated connections |
| `BenchmarkSuite` | Defines individual benchmark workloads |
| `BenchmarkRegistry` | Static registration system for benchmark functions |

## Building

```bash
# Build the server
buck2 build fbcode//thrift/lib/cpp2/fast_thrift/bench:benchmark_server

# Build the client
buck2 build fbcode//thrift/lib/cpp2/fast_thrift/bench:benchmark_client
```

## Running Benchmarks

### 1. Start the Server

```bash
buck2 run fbcode//thrift/lib/cpp2/fast_thrift/bench:benchmark_server -- \
  --port=5001 \
  --io_threads=8
```

**Server Flags:**

| Flag | Default | Description |
|------|---------|-------------|
| `--port` | `5001` | TCP port to listen on |
| `--io_threads` | `0` | Number of IO threads (0 = hardware concurrency) |

### 2. Run the Client

```bash
buck2 run fbcode//thrift/lib/cpp2/fast_thrift/bench:benchmark_client -- \
  --port=5001 \
  --client_threads=8 \
  --runtime_s=30
```

**Client Flags:**

| Flag | Default | Description |
|------|---------|-------------|
| `--port` | (required) | Server port to connect to |
| `--client_threads` | `1` | Number of concurrent client connections (0 = hardware concurrency) |
| `--runtime_s` | `30` | Duration per test in seconds |
| `--test_name` | (all) | Comma-separated list of specific tests to run |

### Example Usage

```bash
# Single-threaded baseline (measures round-trip latency)
buck2 run fbcode//thrift/lib/cpp2/fast_thrift/bench:benchmark_client -- \
  --port=5001 --client_threads=1 --runtime_s=10

# Maximum throughput (auto-detect CPU cores)
buck2 run fbcode//thrift/lib/cpp2/fast_thrift/bench:benchmark_client -- \
  --port=5001 --client_threads=0 --runtime_s=30

# Run specific test only
buck2 run fbcode//thrift/lib/cpp2/fast_thrift/bench:benchmark_client -- \
  --port=5001 --client_threads=8 --test_name=Payload_4KB
```

## Benchmark Tests

The suite includes the following predefined benchmarks:

| Test Name | Payload Size | Description |
|-----------|--------------|-------------|
| `Payload_64B` | 64 bytes | Small payload, tests minimum overhead |
| `Payload_4KB` | 4 KB | Medium payload, typical RPC size |
| `Payload_1MB` | 1 MB | Large payload, tests throughput limits |

## Output Format

```text
=== Payload_4KB (8 threads) ===
  Threads:      8
  QPS:          125000
  Requests:     3750000
  Latency (us):
    P50:    55.23
    P90:    82.15
    P99:    145.67
```

**Metrics:**

| Metric | Description |
|--------|-------------|
| `Threads` | Number of concurrent client connections |
| `QPS` | Total queries per second across all threads |
| `Requests` | Total number of requests completed |
| `P50` | Median latency in microseconds |
| `P90` | 90th percentile latency |
| `P99` | 99th percentile latency |

## Adding Custom Benchmarks

Use the `FAST_THRIFT_BENCHMARK` macro to define new tests in `BenchmarkSuite.cpp`:

```cpp
#include <thrift/lib/cpp2/fast_thrift/bench/BenchmarkRegistry.h>

FAST_THRIFT_BENCHMARK(MyCustomTest) {
  // 'adapter' is a BenchmarkClientAppAdapter& provided by the framework
  auto payload = folly::IOBuf::copyBuffer("test data");
  auto response = co_await adapter.echo(std::move(payload));
  folly::doNotOptimizeAway(response);
}
```

Benchmarks are automatically registered at startup and will appear in the test list.

## Concurrency Model

The `ClientRunner` is modeled after the thrift stresstest framework:

- **Per-thread EventBase**: Each client thread has its own `ScopedEventBaseThread` and TCP connection
- **No thread creation per test**: Benchmark loops run directly on the existing EventBase threads
- **Lock-free latency collection**: Each thread collects latencies independently, aggregated after the run
- **Coordinated start/stop**: Uses `folly::Latch` for connection coordination and `folly::Baton` for completion signaling

This design ensures accurate QPS measurement without thread creation overhead affecting results.

## Tips for Accurate Benchmarking

1. **Warm up the server** by running a short benchmark first before collecting results
2. **Use sufficient runtime** (`--runtime_s=30` or more) to smooth out variance
3. **Match client and server thread counts** for maximum throughput testing
4. **Run on dedicated hardware** to avoid interference from other processes
5. **Consider CPU pinning** for reproducible results on NUMA systems

## Troubleshooting

**Connection refused:**

- Ensure the server is running on the specified port
- Check firewall settings

**Low QPS with many threads:**

- The server may be the bottleneck; increase `--io_threads`
- Check for CPU saturation on the server

**High tail latency:**

- May indicate contention; try reducing `--client_threads`
- Check for background processes competing for resources
