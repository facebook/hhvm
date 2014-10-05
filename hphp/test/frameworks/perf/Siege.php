<?hh

require_once('PerfOptions.php');
require_once('PerfSettings.php');
require_once('Process.php');
require_once('RequestMode.php');
require_once('SiegeStats.php');

final class Siege extends Process {
  use SiegeStats;

  private ?string $logfile;

  public function __construct(
    private PerfOptions $options,
    private PerfTarget $target,
    private RequestMode $mode,
  ) {
    parent::__construct($options->siege);

    if (!$options->skipVersionChecks) {
      $version_line = trim(
        exec(escapeshellarg($options->siege).' --version 2>&1 | head -n 1')
      );
      if (preg_match('/^SIEGE 3\.0\.[0-7]$/', $version_line)) {
        fprintf(
          STDERR,
          "WARNING: Siege 3.0.0-3.0.7 sends an incorrect HOST header to ports ".
          "other than :80 and :443. You are using '%s'.\n\n".
          "You can specify a path to siege 2.7x or >= 3.0.8 with the ".
          "--siege=/path/to/siege option. If you have patched siege to fix ".
          "this issue, pass --skip-version-checks.\n",
          $version_line,
        );
        exit(1);
      }
    }

    if ($mode === RequestModes::BENCHMARK) {
      $this->logfile = tempnam($options->tempDir, 'siege');
    }
  }

  public function __destruct() {
    $logfile = $this->logfile;
    if ($logfile !== null) {
      unlink($logfile);
    }
  }

  public function start(): void {
    parent::start_worker(
      $this->options->daemonOutputFileName('siege'),
      $this->options->delayProcessLaunch,
      $this->options->traceSubProcess,
    );
  }

  protected function getArguments(): Vector<string> {
    $urls_file = tempnam($this->options->tempDir, 'urls');
    $urls = file_get_contents($this->target->getURLsFile());
    $urls = str_replace(
      '__HTTP_PORT__',
      (string) PerfSettings::HttpPort(),
      $urls,
    );
    $urls = str_replace(
      '__HTTP_HOST__',
      gethostname(),
      $urls
    );
    file_put_contents($urls_file, $urls);

    switch ($this->mode) {
      case RequestModes::WARMUP:
        return Vector {
          '-c', (string) PerfSettings::WarmupConcurrency(),
          '-r', (string) PerfSettings::WarmupRequests(),
          '-f', $urls_file,
          '--benchmark',
          '--log=/dev/null',
        };
      case RequestModes::BENCHMARK:
        return Vector {
          '-c', (string) PerfSettings::BenchmarkConcurrency(),
          '-t', PerfSettings::BenchmarkTime(),
          '-f', $urls_file,
          '--benchmark',
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
