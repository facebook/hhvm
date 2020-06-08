<?hh
<<__EntryPoint>>
function main_entry(): void {
  $GLOBALS['x'] = "ok\n";
  echo $GLOBALS['x'];
  include "unset.inc";
  unset_();
  echo $GLOBALS['x'];
}
