<?hh

class C { public static $p; }

<<__EntryPoint, __Rx>>
function test() {
  $x = isset(C::$p);
}
