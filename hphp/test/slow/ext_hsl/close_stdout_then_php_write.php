<?hh // strict

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  $stdout = _OS\request_stdio_fd(_OS\STDOUT_FILENO);
  _OS\write($stdout, "Closing STDOUT\n");
  _OS\close($stdout);

  set_error_handler(
    (int $_code, $message, $_file, $line) ==> {
      \fprintf(\STDERR, "Error: '%s' at line %d\n", $message, $line);
    }
  );

  \fwrite(\STDOUT, "I should fail\n");
  echo "I should also fail\n";
  \fwrite(\STDERR, "STDERRR is still alive\n");
}
