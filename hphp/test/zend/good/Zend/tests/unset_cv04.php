<?hh
<<__EntryPoint>> function f() {
  $GLOBALS['x'] = "ok\n";
  echo $GLOBALS['x'];
  include "unset.inc";
  unset_();
  echo $GLOBALS['x'];
}
