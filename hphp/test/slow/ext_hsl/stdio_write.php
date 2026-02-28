<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  $stdout = _OS\request_stdio_fd(_OS\STDOUT_FILENO);
  $stderr = _OS\request_stdio_fd(_OS\STDERR_FILENO);
  \var_dump($stdout);
  \var_dump($stderr);
  _OS\write($stdout, "Hello STDOUT\n");
  _OS\write($stderr, "Hello STDERR\n");
}
