<?hh

abstract class Process {
  protected ?resource $process;
  protected ?resource $stdin;
  protected ?resource $stdout;
  protected ?string $command;

  private static Vector<Process> $processes = Vector {};

  public function __construct(private string $executablePath) {
    self::$processes[] = $this;
  }

  final public static function cleanupAll() {
    foreach (self::$processes as $process) {
      $process->__destruct();
    }
  }

  abstract protected function getArguments(): Vector<string>;
  protected function getEnvironmentVariables(): Map<string, string> {
    return Map { };
  }

  public function getExecutablePath(): string {
    return $this->executablePath;
  }

  abstract public function start(): void;

  public function start_worker(
    ?string $outputFileName = null,
    float $delayProcessLaunch = 0.1,
    bool $trace = false,
  ): void {
    $executable = $this->getExecutablePath();

    $this->command = $executable.' '.implode(
      ' ',
      $this->getArguments()->map($x ==> escapeshellarg($x)),
    );
    $use_pipe = ($outputFileName === null);
    $spec = [
      0 => ['pipe', 'r'], // stdin
      1 => $use_pipe ? ['pipe', 'w'] : ['file', $outputFileName, 'a'], // stdout
      // not currently using file descriptor 2 (stderr)
    ];
    $pipes = [];
    $env = new Map($_ENV);
    $env->setAll($this->getEnvironmentVariables());

    if ($trace) {
      if ($use_pipe) {
        echo $this->command."\n";
      } else {
        echo $this->command." >> ".$outputFileName."\n";
      }
    }

    $proc = proc_open($this->command, $spec, $pipes, null, $env);

    // Give the shell some time to figure out if it could actually launch the
    // process
    Process::sleepSeconds($delayProcessLaunch);
    invariant(
      $proc && proc_get_status($proc)['running'] === true,
      'failed to start process: %s',
      $this->command
    );

    $this->process = $proc;
    $this->stdin = $pipes[0];
    if ($use_pipe) {
      $this->stdout = $pipes[1];
    }
  }

  public function isRunning(): bool {
    $proc = $this->process;
    if ($proc === null) {
      return false;
    }
    $state = proc_get_status($proc);
    return $state['running'];
  }

  public function stop(): void {
    if (!$this->isRunning()) {
      return;
    }

    if (is_resource($this->stdin)) {
      pclose($this->stdin);
    }
    if (is_resource($this->stdout)) {
      pclose($this->stdout);
    }
    proc_terminate($this->process);
  }

  public function getPid(): ?int {
    $proc = $this->process;
    if ($proc === null) {
      return null;
    }
    $state = proc_get_status($proc);
    if (!$state['running']) {
      return null;
    }
    return $state['pid'];
  }

  public function wait(): void {
    $pid = $this->getPid();
    if ($pid === null) {
      return;
    }
    $status = null;
    pcntl_waitpid($pid, $status);
  }

  public function __destruct() {
    if ($this->isRunning()) {
      $this->stop();
    }
  }

  public static function sleepSeconds(float $secs): void {
    usleep($secs * 1e06);
  }
}
