<?hh

class C { public static $p; }

<<__EntryPoint>>
function test()[rx] {
  $x = isset(C::$p);
}
