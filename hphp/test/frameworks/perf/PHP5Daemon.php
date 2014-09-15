<?hh

require_once('NoEngineStats.php');
require_once('PerfSettings.php');
require_once('PHPEngine.php');

final class PHP5Daemon extends PHPEngine {
  use NoEngineStats;

  public function __construct(
    private string $tempDir,
    private PerfTarget $target,
    string $executable_path,
  ) {
    parent::__construct($executable_path);
  }

  protected function getArguments(): Vector<string> {
    return Vector {
      '-b', '127.0.0.1:'.PerfSettings::FastCGIPort(),
      '-c', __DIR__,
    };
  }

  protected function getEnvironmentVariables(): Map<string, string> {
    return Map {
      'PHP_FCGI_CHILDREN' => '60',
      'PHP_FCGI_MAX_REQUESTS' => '0',
    };
  }
}
