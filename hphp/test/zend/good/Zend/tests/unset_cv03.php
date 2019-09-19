<?hh
<<__EntryPoint>>
function main_entry(): void {
  $x = "ok\n";
  echo $x;
  include "unset.inc";
  echo $x;
}
