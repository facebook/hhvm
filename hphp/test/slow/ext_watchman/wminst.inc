<?hh

// Borrowed and stripped down from Watchman project test infrastructure.
class WatchmanInstance {
  private $proc;
  private $logfile;
  private $sockname;
  private $config_file;
  private $repo_root;
  private $run_details = '';
  const TIMEOUT = 60;
  private function tempfile() {
    $temp = tempnam(sys_get_temp_dir(), 'wat');
    return $temp;
  }
  function __construct($tmpdir) {
    // Not atomic but PHP doesn't support this. Should be fine.
    $this->repo_root = $tmpdir;
    $this->logfile = $this->tempfile();
    $this->sockname = tempnam(getenv('HPHP_TEST_SOCKETDIR'), 'wat');
    $this->config_file = $this->tempfile();
    $config_json = '{}';
    file_put_contents($this->config_file, $config_json);
    $this->start();
  }
  function getFullSockName() {
    return $this->sockname . '.sock';
  }
  function getRepoRoot() {
    return $this->repo_root;
  }
  private function start() {
    $cmd = "watchman --foreground --sockname=%s --logfile=%s " .
            "--statefile=%s.state --log-level=2 --pidfile=%s";
    putenv("WATCHMAN_CONFIG_FILE=".$this->config_file);
    $cmd = sprintf(
      $cmd,
      $this->getFullSockName(),
      $this->logfile,
      $this->logfile,
      $this->tempfile(),
    );
    $pipes = darray[];
    $this->run_details = "Ran command: $cmd, in ".$this->repo_root;
    $this->proc = proc_open($cmd, darray[
      0 => varray['file', '/dev/null', 'r'],
      1 => varray['file', $this->logfile, 'a'],
      2 => varray['file', $this->logfile, 'a'],
    ], inout $pipes, $this->repo_root);
    if (!$this->proc) {
      throw new Exception("Failed to spawn $cmd");
    }
    try {
      $this->openSock();
    } catch(Exception $e) {
      $this->dumpLog();
      throw $e;
    }
  }
  public function dumpLog() {
    print("Dumping log:\n");
    $return_var = -1;
    system("cat $this->logfile", inout $return_var);
    print($this->run_details."\n");
  }
  private function openSock() {
    $sockname = $this->getFullSockName();
    $deadline = time() + 60;
    do {
      if (!file_exists($sockname)) {
        usleep(30000);
      }
      $errno = null;
      $errstr = null;
      $this->sock = @fsockopen('unix://' . $sockname, -1, inout $errno, inout $errstr);
      if ($this->sock) {
        break;
      }
    } while (time() <= $deadline);
    if (!$this->sock) {
      throw new Exception("Failed to talk to watchman on $sockname");
    }
    stream_set_timeout($this->sock, self::TIMEOUT);
  }
  private function waitForTerminate($timeout) {
    if (!$this->proc) {
      return false;
    }
    $deadline = time() + $timeout;
    do {
      $st = proc_get_status($this->proc);
      if (!$st['running']) {
        return $st;
      }
      usleep(250000);
    } while (time() <= $deadline);
    return $st;
  }
  function terminateProcess() {
    if (!$this->proc) {
      return;
    }
    proc_terminate($this->proc);
    $st = $this->waitForTerminate(self::TIMEOUT);
    if ($st['running']) {
      proc_terminate($this->proc, 9);
      $st = $this->waitForTerminate(15);
      $this->proc = null;
    }
  }
}
