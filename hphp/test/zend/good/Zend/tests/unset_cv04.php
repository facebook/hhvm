<?hh
<<__EntryPoint>> function f() {
  $x = "ok\n";
  echo $x;
  include "unset.inc";
  echo $x;
}
