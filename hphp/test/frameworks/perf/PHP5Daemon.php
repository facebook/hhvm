<?hh

require_once('NoEngineStats.php');
require_once('PerfOptions.php');
require_once('PerfSettings.php');
require_once('PHPEngine.php');

final class PHP5Daemon extends PHPEngine {
  use NoEngineStats;

  public function __construct(
    private PerfOptions $options,
    private PerfTarget $target,
  ) {
    parent::__construct($this->options->php5);
  }

  public function start(): void {
    parent::start($this->options->daemonOutputFileName('php5'),
                  $this->options->delayProcessLaunch,
                  $this->options->traceSubProcess);
  }

  protected function getArguments(): Vector<string> {
    return Vector {
      '-b', '127.0.0.1:'.PerfSettings::FastCGIPort(),
    };
  }

  protected function getEnvironmentVariables(): Map<string, string> {
    return Map {
      'PHP_FCGI_CHILDREN' => '60',
      'PHP_FCGI_MAX_REQUESTS' => '0',
    };
  }
}
