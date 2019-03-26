<?hh

class C { public static $p; public static $q; }

<<__Rx>>
function test() {
  // global $a; // VGetG (global statement)

  $y = 42;
  $_GET =& $y;           // VGetL, BindG (superglobals)

  // the following VGetG is covered by
  // $GLOBALS elements may not be taken by reference
  // $b = &$GLOBALS['foo']; // VGetG (globals array)

  // the following VGetG are covered by
  // Fatal error: Superglobals may not be taken by reference
  // $z = &$_POST;
  // f(&$_POST);

  C::$p =& C::$q; // VGetS, BindS

  $o = new stdClass();
  $o->p =& $o->q; // VGetM, BindM
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
