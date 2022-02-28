<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  $true = file_exists('/bin/true') ? '/bin/true' : '/usr/bin/true';
  invariant(
    file_exists($true),
    "Could not find path to 'true', add path to test"
  );
  print "--- Full path, execve\n";
  _OS\fork_and_execve(
    $true,
    vec[],
    vec[],
    dict[],
    shape(),
  );
  print "--- Full path, execvpe\n";
  _OS\fork_and_execve(
    $true,
    vec[],
    vec[],
    dict[],
    shape('execvpe' => true),
  );
  print "--- Depend on \$PATH, execve (expected failure)\n";
  try {
  _OS\fork_and_execve(
    'true',
    vec[],
    vec[],
    dict[],
    shape(),
  );
  } catch (_OS\ErrnoException $e) {
    printf(
      "  Exception: %s (%d - %s)\n",
      $e->getMessage(),
      $e->getCode(),
      posix_strerror($e->getCode()),
    );
  }
  print "--- Depend on \$PATH, execvpe\n";
  _OS\fork_and_execve(
    'true',
    vec[],
    vec['PATH='.dirname($true)],
    dict[],
    shape('execvpe' => true),
  );
  print "--- Check environment variables are preserved with execvpe\n";
  list($ro, $wo) = _OS\pipe();
  list($re, $we) = _OS\pipe();
  $pid = _OS\fork_and_execve(
    '/bin/sh',
    vec['/bin/sh', '-c', 'echo $FOO; echo $BAR >&2'],
    vec['FOO=HERP', 'BAR=DERP'],
    dict[_OS\STDOUT_FILENO => $wo,
    _OS\STDERR_FILENO => $we,],
    shape('execvpe' => true),
  );
  _OS\close($wo);
  _OS\close($we);
  $status = null;
  pcntl_waitpid($pid, inout $status);
  printf("STDOUT: %s", _OS\read($ro, 1024));
  printf("STDERR: %s", _OS\read($re, 1024));
  _OS\close($ro);
  _OS\close($re);

}
