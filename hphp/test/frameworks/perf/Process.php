<?hh

abstract class Process {
  protected ?resource $process;
  protected ?resource $stdin;
  protected ?resource $stdout;

  public function __construct(private string $executablePath) {
  }

  abstract protected function getArguments(): Vector<string>;
  protected function getEnvironmentVariables(): Map<string, string> {
    return Map { };
  }

  public function getExecutablePath(): string {
    return $this->executablePath;
  }

  public function start(): void {
    $executable = $this->getExecutablePath();

    $cmd = $executable.' '.implode(
      ' ',
      $this->getArguments()->map($x ==> escapeshellarg($x)),
    );
    $spec = [
      0 => ['pipe', 'r'], // stdin
      1 => ['pipe', 'w'], // stdout
      // not currently using 2 (stderr)
    ];
    $pipes = [];
    $env = new Map($_ENV);
    $env->setAll($this->getEnvironmentVariables());
    $proc = proc_open($cmd, $spec, $pipes, null, $env);

    // Give the shell some time to figure out if it could actually launch the
    // process
    usleep(100000 /* = 100ms */);
    invariant(
      $proc && proc_get_status($proc)['running'] === true,
      'failed to start process: %s',
      $cmd
    );

    $this->process = $proc;
    $this->stdin = $pipes[0];
    $this->stdout = $pipes[1];
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

    pclose($this->stdin);
    pclose($this->stdout);
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
}
