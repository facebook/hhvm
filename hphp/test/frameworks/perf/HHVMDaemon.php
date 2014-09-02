<?hh

require_once('HHVMStats.php');
require_once('PerfSettings.php');
require_once('PHPEngine.php');

final class HHVMDaemon extends PHPEngine {
  use HHVMStats;

  public function __construct(
    private string $tempDir,
    private PerfTarget $target,
    string $executable_path
  ) {
    parent::__construct($executable_path);
  }

  protected function getTarget(): PerfTarget {
    return $this->target;
  }

  protected function getArguments(): Vector<string> {
    return Vector {
      '-m', 'server',
      '-p', (string) PerfSettings::FastCGIPort(),
      '-v', 'Server.Type=fastcgi',
      '-v', 'Eval.Jit=1',
      '-v', 'AdminServer.Port='.PerfSettings::FastCGIAdminPort(),
    };
  }

  public function start(): void {
    parent::start();
    assert($this->isRunning());
    for ($i = 0; $i < 10; ++$i) {
      sleep(1);
      $health = $this->adminRequest('/check-health');
      if ($health) {
        $health = json_decode($health, /* assoc array = */ true);
        if (array_key_exists('tc-size', $health) && $health['tc-size'] > 0) {
          return;
        }
      }
      $this->stop();
      return;
    }
  }

  public function stop(): void {
    $health = $this->adminRequest('/check-health');
    if ($health && json_decode($health)) {
      $this->adminRequest('/stop');
      assert(!$this->isRunning());
    } else {
      parent::stop();
    }
  }

  protected function adminRequest(string $path): string {
    $result = file_get_contents(
      'http://localhost:'.PerfSettings::HttpAdminPort().$path
    );
    invariant($result !== false, 'Admin request failed');
    return $result;
  }
}
