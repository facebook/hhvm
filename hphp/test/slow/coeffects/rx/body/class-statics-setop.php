<?hh

class C { public static $p; }

<<__EntryPoint>>
function test()[rx, globals] {
  C::$p *= 2;
}
