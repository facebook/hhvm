<?hh

require_once('HHVMStats.php');
require_once('PerfOptions.php');
require_once('PerfSettings.php');
require_once('PHPEngine.php');
require_once('Process.php');

final class HHVMDaemon extends PHPEngine {
  use HHVMStats;

  public function __construct(
    private PerfOptions $options,
    private PerfTarget $target,
  ) {
    parent::__construct((string) $options->hhvm);

    if ($options->notBenchmarking) {
      return;
    }
    $output = [];
    exec(
      implode(
        ' ',
        (Vector {
            $options->hhvm,
            '-v', 'Eval.Jit=1',
            __DIR__.'/hhvm_config_check.php',
        })->map($x ==> escapeshellarg($x))
      ),
      $output
    );
    $checks = json_decode(implode("\n", $output), /* as array = */ true);
    invariant($checks, 'Got invalid output from hhvm_config_check.php');
    $failed = 0;
    foreach ($checks as $name => $data) {
      if (!$data['OK']) {
        $failed++;
        fprintf(
          STDERR,
          "HHVM build is not suitable for benchmarking:\n".
          "  %s: %s\n".
          "  Required: %s\n",
          $name,
          $data['Value'],
          $data['Required Value'],
        );
      }
    }
    if ($failed !== 0) {
      fwrite(
        STDERR,
        "Exiting due to invalid config. You can run anyway with ".
        "--i-am-not-benchmarking, but the results will not be suitable for ".
        "any kind of comparison.\n"
      );
      exit(1);
    }
  }

  protected function getTarget(): PerfTarget {
    return $this->target;
  }

  protected function getArguments(): Vector<string> {
    $args = Vector {
      '-m', 'server',
      '-p', (string) PerfSettings::FastCGIPort(),
      '-v', 'Server.Type=fastcgi',
      '-v', 'Eval.Jit=1',
      '-v', 'AdminServer.Port='.PerfSettings::FastCGIAdminPort(),
    };
    if (strlen($this->options->hhvmExtraArguments) > 0) {
      //
      // The ice is very thin here regarding this use of explode,
      // as the arguments' values may themselves contain spaces.
      //
      $arrayExtras = explode(' ', trim($this->options->hhvmExtraArguments), 1000);
      $args->addAll($arrayExtras);
    }
    return $args;
  }

  public function start(): void {
    parent::start($this->options->daemonOutputFileName('hhvm'),
                  $this->options->delayProcessLaunch,
                  $this->options->traceSubProcess);
    invariant($this->isRunning(), 'Failed to start HHVM');
    for ($i = 0; $i < 10; ++$i) {
      Process::sleepSeconds($this->options->delayCheckHealth);
      $health = $this->adminRequest('/check-health', true);
      if ($health) {
        if ($health == "failure") {
          continue;
        }
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
    try {
      $health = $this->adminRequest('/check-health');
      if ($health && json_decode($health)) {
        $this->adminRequest('/stop');
        invariant(!$this->isRunning(), 'Failed to stop HHVM');
      }
    } catch (Exception $e) {
      parent::stop();
    }
  }

  protected function adminRequest(string $path, $allowFailures = true): string {
    $url = 'http://localhost:'.PerfSettings::HttpAdminPort().$path;
    $ctx = stream_context_create(
      ['http' => ['timeout' => $this->options->maxdelayAdminRequest]]
    );
    //
    // TODO: it would be nice to suppress
    // Warning messages from file_get_contents
    // in the event that the connection can't even be made.
    //
    $result = file_get_contents(
      $url,
      /* include path = */ false,
      $ctx);
    if ($result != false) {
      return $result;
    }
    if ($allowFailures) {
      return "failure";
    } else {
      invariant($result !== false, 'Admin request failed');
      return $result;
    }
  }
}
