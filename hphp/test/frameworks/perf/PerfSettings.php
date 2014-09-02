<?hh

final class PerfSettings {

  ///// Benchmark Settings /////

  // Per concurrent thread - so, total number of requests made during warmup
  // is WarmupRequests * WarmupConcurrency
  public static function WarmupRequests(): int {
    return 30;
  }

  public static function WarmupConcurrency(): int {
    return 10;
  }

  public static function BenchmarkTime(): string{
    // [0-9]+[SMH]
    return '1M'; // 1 minute
  }

  public static function BenchmarkConcurrency(): int {
    return 60;
  }

  ///// Server Settings /////

  public static function HttpPort(): int {
    return 8080;
  }

  public static function HttpAdminPort(): int {
    return 8081;
  }

  public static function FastCGIPort(): int {
    return 8082;
  }

  public static function FastCGIAdminPort(): int {
    return 8100;
  }
}
