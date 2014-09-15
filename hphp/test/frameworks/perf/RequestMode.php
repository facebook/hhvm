<?hh

newtype RequestMode = string;

final class RequestModes {
  const RequestMode WARMUP = 'warmup';
  const RequestMode BENCHMARK = 'benchmark';
}
