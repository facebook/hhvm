<?hh

abstract final class Sweeper {
  private static ?vec<IDisposable> $pending = null;

  public static function add(IDisposable $d): int {
    if (self::$pending === null) {
      self::$pending = vec[];

      set_error_handler(() ==> {
        foreach (self::$pending as $p) if ($p !== null) $p->__dispose();
        return false;
      }, E_ERROR|E_RECOVERABLE_ERROR);
    }

    self::$pending[] = $d;
    return count(self::$pending) - 1;
  }

  public static function remove(int $index): void {
    self::$pending[$index] = null;
  }

  public static function mkdtemp(string $name): string {
    return HH\Lib\_Private\_OS\mkdtemp(ServerUtilServerTests::working_dir().'/'.$name);
  }

  public static function removeDir(string $dir): void {
    if (strpos($dir, ServerUtilServerTests::working_dir()) !== 0) return;

    $files = scandir($dir);
    foreach ($files as $file) {
      if ($file === '.' || $file === '..') continue;
      $p = $dir.'/'.$file;
      if (is_dir($p)) self::removeDir($p);
      else unlink($p);
    }
  }

}

class TestWebRoot implements IDisposable {
  private string $webRoot;
  private vec<dict<string, string>> $fileVersions = vec[];
  private ?int $index = null;
  private int $curVersion = -1;

  private static ?dict<string, TestWebRoot> $pending = null;

  public function __construct(private string $testName, int $id) {
    $this->webRoot = Sweeper::mkdtemp('hhvm-multipart-XXXXXX');
    $testData = file_get_contents(__DIR__."/../multipart/$testName");

    $matches = preg_split(
      '|\n// (FILE: [a-zA-Z0-9_/.-]+)(?: (VERSION: [0-9]+))?\n|',
      $testData,
      -1,
      PREG_SPLIT_DELIM_CAPTURE
    );

    $file = null;
    for ($i = 0; $i < count($matches); $i++) {
      if (($f = sscanf($matches[$i], 'FILE: %s')) === null) continue;
      $file = $f[0];

      if (($f = sscanf($matches[++$i], 'VERSION: %d')) !== null) {
        $vers = $f[0];
        $i++;
      } else $vers = 0;

      while ($vers >= count($this->fileVersions)) {
        $this->fileVersions[] = dict[];
      }

      $this->fileVersions[$vers][$file] = $matches[$i];
    }

    $this->putFile('.watchmanconfig', '{}');
    $this->putFile( // May be overriden if the test specifies a custom config
      '.hhvmconfig.hdf',
      'Autoload.Query = {'.
        '"suffix":"php",'.
        '"expression":["allof",["type","f"],["suffix","php"]]'.
      '}'
    );
    $this->putFile( // The test framework expects this special "status" file
      'hello.php',
      file_get_contents(__DIR__.'/../server_root/hello.php'),
    );
    $this->putFile(
      'sandbox-id.txt',
      "sandbox$id",
    );
    $this->update(0);
    $this->watchman("watch ".$this->webRoot);
    $this->index = Sweeper::add($this);
  }

  public function __dispose(): void {
    if ($this->index !== null) {
      $this->watchman("watch-del ".$this->webRoot);
      Sweeper::removeDir($this->webRoot);
      Sweeper::remove($this->index);
      $this->index = null;
    }
  }

  private function watchman(string $cmd): void {
    $result = null;
    $ret    = -1;
    exec("watchman $cmd", inout $result, inout $ret);
  }

  private function putFile(string $name, string $contents): void {
    file_put_contents($this->webRoot."/$name", $contents);
  }

  public function update(?int $vers = null): void {
    if ($vers === null) {
      $vers = min($this->cur() + 1, $this->versions() - 1);
    }
    if ($vers === $this->curVersion) return;

    for ($v = $this->curVersion + 1; $v <= $vers; $v++) {
      foreach ($this->fileVersions[$v] as $file => $contents) {
        $header = "";
        $phplen = strlen(".php");
        if (strlen($file) >= $phplen && substr($file, -$phplen) === ".php") {
          $header = "<?hh\n\n";
        }
        $this->putFile($file, $header.$contents);
      }
    }

    $this->curVersion = $vers;
    if ($vers) usleep(1000000); // Let the filesystem / watchman catch up
  }

  public function cur(): int { return $this->curVersion; }
  public function dir(): string { return $this->webRoot; }
  public function versions(): int { return count($this->fileVersions); }
}

class Sandbox {
  public function __construct(
    private string $hostPrefix,
    private int $hostPort,
    private int $index,
    private TestWebRoot $root,
  ) {}

  public function index(): int { return $this->index; }
  public function root(): TestWebRoot { return $this->root; }
  public function request(
    mixed $path,
    int $timeout = 1200,
    string $curl_opts = '',
  ) {
    return http_request(
      'localhost',
      $this->hostPort,
      $path,
      $timeout,
      $curl_opts,
      $this->hostPrefix,
    );
  }
}

class TestWebServer implements IDisposable {
  private vec<TestWebRoot> $roots = vec[];
  private string $home = '';
  private mixed $serverProc = null;
  private ?int $index = null;
  private ?int $serverPort = null;
  private ?int $adminPort = null;

  public function __construct(string $testName, int $numRoots = 1) {
    $this->home = Sweeper::mkdtemp('hhvm-test-home-XXXXXX');
    for ($i = 0; $i < $numRoots; $i++) {
      $this->roots[] = new TestWebRoot($testName, $i);
      mkdir($this->home."/sandbox$i");
      file_put_contents(
        $this->home."/sandbox$i/sandbox_conf.hdf",
        "default.path = ".$this->roots[$i]->dir()
      );
    }
    $this->index = Sweeper::add($this);
  }

  public function __dispose(): void {
    if ($this->index !== null) {
      foreach ($this->roots as $twr) $twr->__dispose();
      Sweeper::removeDir($this->home);
      Sweeper::remove($this->index);
      if ($this->serverProc !== null) $this->killServer();
      $this->index = null;
    }
  }

  private function getSandboxes(): vec<Sandbox> {
    $sboxes = vec[];
    for ($i = 0; $i < count($this->roots); $i++) {
      $sboxes[] = new Sandbox(
        "www.sandbox$i",
        $this->serverPort,
        $i,
        $this->roots[$i]
      );
    }
    return $sboxes;
  }

  public function withServer(
    (function (vec<Sandbox>): void) $f,
    string $customArgs = '',
    string $repoArgs = '',
  ): void {
    try {
      $this->startServer($customArgs, $repoArgs);
      $f($this->getSandboxes());
      $this->stopServer();
    } catch (Exception $e) {
      $pid = posix_getpid();
      error_log(
        "Caught exception, test failed, pid=$pid, exn=\n".(string)$e
      );
      $this->killServer();
      exit(-1);
    }
  }

  private function startServer(string $customArgs, string $repoArgs): void {
    $serverPort = $adminPort = null;
    $debugPort = false;
    $args = implode($this->serverOptions(), ' ')." $customArgs";

    $this->serverProc = startServer(
      inout $serverPort,
      inout $adminPort,
      inout $debugPort,
      $this->home,
      $this->roots[0]->dir(),
      $args,
      null,
      $repoArgs,
    );

    $this->adminPort = $adminPort;
    $this->serverPort = $serverPort;
  }

  private function stopServer(): void {
    if ($this->serverProc !== null) {
      stopServer($this->adminPort, $this->serverProc);
      $this->serverProc = null;
    }
  }

  private function killServer(): void {
    killChildren(posix_getpid());
    if ($this->serverProc !== null) {
      proc_close($this->serverProc);
      $this->serverProc = null;
    }
    error_log('test failed');
  }

  private function serverOptions(): vec<string> {
    return vec[
      '-vwatchman.enable=true',
      '-vAutoload.DB.CanCreate=true',
      '-vAutoload.DB.Path='.$this->home.'/facts_%{schema}.sql3',
      '-vSandbox.ConfFile=sandbox_conf.hdf',
      '-vSandbox.Pattern="www\.([a-z0-9_-]+)\.[a-z0-9_-]+\.[a-z0-9_-]+"',
      '-vSandbox.SandboxMode=true',
      '-vEval.EnableIntrinsicsExtension=1',
      '-vEval.BespokeArrayLikeMode=0',
    ];
  }
}

function runSandboxTest(
  (function (vec<Sandbox>): void) $f,
  ?string $file = null,
  int $count = 3,
  string $customArgs = '',
  string $repoArgs = '',
) {
  require __DIR__ . '/../../util/server_tests.inc';
  ServerUtilServerTests::$LOG_ROOT = ServerUtilServerTests::working_dir() . '/hhvm_server';

  if ($file === null) {
    $file = basename(hphp_debug_caller_info()['file']);
  }
  using $server = new TestWebServer($file, $count);
  $server->withServer($f, $customArgs, $repoArgs);
}

function runDefaultSandboxTest(
  int $count = 3,
  string $customArgs = '',
  string $repoArgs = '',
  string $main = 'main.php',
  ?string $test = null,
) {
  runSandboxTest(
    ($sandboxList) ==> {
      if (getenv('PAUSE_SERVER_TESTS') !== false) readline("Next ?");
      for ($i = 0; $i < $sandboxList[0]->root()->versions(); $i++) {
        foreach ($sandboxList as $sandbox) {
          if ($sandbox->index() % 3 == 2) {
            $sandbox->root()->update($sandbox->root()->versions() - 1);
          }
          $id = $sandbox->request('sandbox-id.txt');
          $s = $sandbox->index();
          $v = $sandbox->root()->cur();
          echo "Sandbox: $s ($id) Version: $v\n";
          echo "--------------------------------------------------------\n";
          echo $sandbox->request($main) ?? "<NULL>";
          echo "\n--------------------------------------------------------\n\n";
          if ($sandbox->index() % 3 == 0) $sandbox->root()->update();
        }
        if (getenv('PAUSE_SERVER_TESTS') !== false) readline("Next ?");
      }
    },
    $test ?? basename(hphp_debug_caller_info()['file']),
    $count,
    $customArgs,
    $repoArgs,
  );
}
