<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  $true = file_exists('/bin/true') ? '/bin/true' : '/usr/bin/true';
  invariant(
    file_exists($true),
    "Could not find path to 'true', add path to test"
  );
  $dev_null = _OS\open('/dev/null', _OS\O_RDWR);
  echo "----------\n"; // handy in CLI server with --count 3
  print("--- Full path, nothing else\n");
  _OS\fork_and_execve($true, vec[], vec[], dict[], shape());
  print("--- Relative path, good cwd\n");
  _OS\fork_and_execve("./true", vec[], vec[], dict[], shape('cwd' => dirname($true)));
  print("--- Using shell exec\n");
  _OS\fork_and_execve(
    "/bin/sh",
    vec['/bin/sh', '-c', 'true'],
    vec["PATH=/bin"],
    dict[],
    shape(),
  );

  print("--- Expecting BAR\n");
  list($r, $w) = _OS\pipe();
  $pid = _OS\fork_and_execve(
    "/bin/sh",
    vec['/bin/sh', '-c', 'echo $FOO'],
    vec['PATH=/bin', 'FOO=BAR'],
    dict[_OS\STDOUT_FILENO => $w],
    shape(),
  );
  _OS\close($w);
  $status = 0;
  pcntl_waitpid($pid, inout $status);
  print(_OS\read($r, 1024));
  _OS\close($r);

  print("--- Not in PATH\n");
  list($r, $w) = _OS\pipe();
  $pid = _OS\fork_and_execve(
    "/bin/sh",
    vec['/bin/sh', '-c', 'cat'],
    vec['PATH='],
    dict[_OS\STDERR_FILENO => $w],
    shape(),
  );
  _OS\close($w);
  pcntl_waitpid($pid, inout $status);
  printf("STDERR: %s", _OS\read($r, 1024));
  _OS\close($r);

  print("--- Bad STDOUT\n");
  list($r, $w) = _OS\pipe();
  $pid = _OS\fork_and_execve(
    '/bin/echo',
    vec['/bin/echo', 'foo'],
    vec[],
    dict[_OS\STDERR_FILENO => $w],
    shape(),
  );
  _OS\close($w);
  pcntl_waitpid($pid, inout $status);
  printf("STDERR: %s", _OS\read($r, 1024));
  _OS\close($r);

  print("--- Custom ARGV[0]\n");
  list($r, $w) = _OS\pipe();
  $pid = _OS\fork_and_execve(
    '/bin/sh',
    vec['myshell', '-c', 'echo $0'],
    vec[],
    dict[_OS\STDIN_FILENO => $dev_null, _OS\STDOUT_FILENO => $w, _OS\STDERR_FILENO => $dev_null],
    shape(),
  );
  _OS\close($w);
  pcntl_waitpid($pid, inout $status);
  printf("STDERR: %s", _OS\read($r, 1024));
  _OS\close($r);

  print("--- Explicitly write to different handles\n");
  list($or, $ow) = _OS\pipe();
  list($er, $ew) = _OS\pipe();
  list($fd42r, $fd42w) = _OS\pipe();
  $pid = _OS\fork_and_execve(
    '/bin/sh',
    vec['/bin/sh', '-c', 'echo Foo; echo Bar >&2; echo Baz >&42'],
    vec[],
    dict[
      _OS\STDOUT_FILENO => $ow,
      _OS\STDERR_FILENO => $ew,
      42 => $fd42w,
    ],
    shape(),
  );
  _OS\close($ow);
  _OS\close($ew);
  _OS\close($fd42w);
  pcntl_waitpid($pid, inout $status);
  // intentionally not reading these in the order they were written.
  printf("STDERR: %s", _OS\read($er, 1024));
  printf("STDOUT: %s", _OS\read($or, 1024));
  printf("FD 42: %s", _OS\read($fd42r, 1024));
  _OS\close($or);
  _OS\close($er);
  _OS\close($fd42r);

  print("--- Good STDIO\n");
  list($ir, $iw) = _OS\pipe();
  list($or, $ow) = _OS\pipe();
  list($er, $ew) = _OS\pipe();
  _OS\write($iw, "Hello, world\n");
  _OS\close($iw);
  $pid = _OS\fork_and_execve(
    '/bin/cat',
    vec['/bin/cat'],
    vec[],
    dict[
      _OS\STDIN_FILENO => $ir,
      _OS\STDOUT_FILENO => $ow,
      _OS\STDERR_FILENO => $ew,
    ],
    shape(),
  );
  _OS\close($ir);
  _OS\close($ow);
  _OS\close($ew);
  pcntl_waitpid($pid, inout $status);
  printf("STDOUT: %s", _OS\read($or, 1024));
  printf("STDERR: %s\n", _OS\read($er, 1024)); // empty
  _OS\close($or);
  _OS\close($er);

  print("--- Working directory does not exist\n");
  try {
    _OS\fork_and_execve(
      $true,
      vec[],
      vec[],
      dict[],
      shape('cwd' => '/idonotexist'),
    );
  } catch (_OS\ErrnoException $e) {
    printf(
      "  Exception: %s (%d - %s)\n",
      $e->getMessage(),
      $e->getCode(),
      posix_strerror($e->getCode()),
    );
  }
}
