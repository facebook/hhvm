<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  $stdout = _OS\request_stdio_fd(_OS\STDOUT_FILENO);
  _OS\write($stdout, "Closing STDOUT\n");
  _OS\close($stdout);
  $stderr = _OS\request_stdio_fd(_OS\STDERR_FILENO);
  _OS\write($stderr, "Closing STDERR\n");
  _OS\close($stderr);
  invariant_violation(
    "There should be no visible error for this, just non-zero exit"
  );
}
