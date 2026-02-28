<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  list ($r, $w1) = _OS\pipe();
  $w2 = _OS\dup($w1);
  _OS\write($w2, "Hello, ");
  _OS\write($w1, "world.\n");

  var_dump(_OS\read($r, 0xff));
  _OS\close($w1);
  _OS\write($w2, "Hello again.\n");
  _OS\close($w2);
  var_dump(_OS\read($r, 0xff));
  var_dump(_OS\read($r, 0xff));
}
