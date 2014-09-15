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
    return 8090;
  }

  public static function HttpAdminPort(): int {
    return 8091;
  }

  public static function FastCGIPort(): int {
    return 8092;
  }

  public static function FastCGIAdminPort(): int {
    return 8093;
  }

  public static function Validate(): void {
    self::CheckPortAvailability();
  }

  public static function CheckPortAvailability(): void {
    $ports = Vector {
      self::HttpPort(),
      self::HttpAdminPort(),
      self::FastCGIPort(),
      self::FastCGIAdminPort(),
    };
    $busy_ports = Vector { };
    foreach ($ports as $port) {
      $result = @fsockopen('localhost', $port);
      if ($result !== false) {
        fclose($result);
        $busy_ports[] = $port;
      }
    }
    if ($busy_ports) {
      fprintf(
        STDERR,
        "Ports %s are required, but already in use. You can find out what ".
        "processes are using them with:\n  sudo lsof -P %s\n",
        implode(', ', $busy_ports),
        implode(' ', $busy_ports->map($x ==> '-i :'.$x)),
      );
      exit(1);
    }
  }
}
