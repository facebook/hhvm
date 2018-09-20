<?hh

use function HH\Lib\_Private\Native\pipe;

<<__EntryPoint>>
function main(): void {
  list($r, $w) = pipe();

  \fwrite($w, "Hello, world!\n");
  \fclose($w);
  \var_dump(\stream_get_contents($r));
}
