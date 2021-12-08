<?hh

class C { public static $p; }

<<__EntryPoint>>
function test()[rx, read_globals] {
  $x = C::$p;
}
