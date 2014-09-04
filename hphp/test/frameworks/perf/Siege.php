<?hh

require_once('PerfSettings.php');
require_once('Process.php');
require_once('RequestMode.php');
require_once('SiegeStats.php');

final class Siege extends Process {
  use SiegeStats;

  private ?string $logfile;

  public function __construct(
    private string $tempDir,
    private PerfTarget $target,
    private RequestMode $mode,
  ) {
    parent::__construct('siege');
    if ($mode === RequestModes::BENCHMARK) {
      $this->logfile = tempnam($tempDir, 'siege');
    }
  }

  public function __destruct() {
    $logfile = $this->logfile;
    if ($logfile !== null) {
      unlink($logfile);
    }
  }

  protected function getArguments(): Vector<string> {
    $urls_file = tempnam($this->tempDir, 'urls');
    $urls = file_get_contents($this->target->getURLsFile());
    $urls = str_replace(
      '__HTTP_PORT__',
      (string) PerfSettings::HttpPort(),
      $urls,
    );
    file_put_contents($urls_file, $urls);

    switch ($this->mode) {
      case RequestModes::WARMUP:
        return Vector {
          '-c', (string) PerfSettings::WarmupConcurrency(),
          '-r', (string) PerfSettings::WarmupRequests(),
          '-f', $urls_file,
          '--log=/dev/null',
        };
      case RequestModes::BENCHMARK:
        return Vector {
          '-c', (string) PerfSettings::BenchmarkConcurrency(),
          '-t', PerfSettings::BenchmarkTime(),
          '-f', $urls_file,
          '--log='.$this->logfile,
        };
      default:
        invariant_violation(
          'Unexpected request mode: '.(string)$this->mode
        );
    }
  }

  protected function getLogFilePath(): string {
    $logfile = $this->logfile;
    invariant(
      $logfile !== null,
      'Tried to get log file path without a logfile'
    );
    return $logfile;
  }
}
