<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  $stdout = _OS\request_stdio_fd(_OS\STDOUT_FILENO);
  _OS\write($stdout, "Closing STDOUT\n");
  \fclose(\HH\stdout());
  try {
    // should fail
    _OS\write($stdout, "BUT I SURVIVED! (bad)\n");
  } catch (_OS\ErrnoException $e) {
    $stderr = _OS\request_stdio_fd(_OS\STDERR_FILENO);
    if ($e->getCode() === _OS\EBADF) {
      _OS\write($stderr, "EBADF (expected)\n");
      return;
    }
    _OS\write($stderr, $e->getMessage()."\n");
  }
}
