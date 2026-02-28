<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  $stdout = _OS\request_stdio_fd(_OS\STDOUT_FILENO);
  _OS\write($stdout, "Closing STDOUT\n");
  _OS\close($stdout);

  set_error_handler(
    (int $_code, $message, $_file, $line) ==> {
      \fprintf(\HH\stderr(), "Error: '%s' at line %d\n", $message, $line);
    }
  );

  \fwrite(\HH\stdout(), "I should fail\n");
  echo "I should also fail\n";
  \fwrite(\HH\stderr(), "STDERRR is still alive\n");
}
