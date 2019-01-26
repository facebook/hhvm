<?hh

class C { public static $p; public static $q; }

<<__Rx>>
function test() {
  $_GET =& $_POST; // VGetG, BindG (superglobals)

  C::$p =& C::$q; // VGetS, BindS

  $o = new stdClass();
  $o->p =& $o->q; // VGetM, BindM
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
