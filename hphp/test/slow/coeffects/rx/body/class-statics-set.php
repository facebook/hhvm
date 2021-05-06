<?hh

class C { public static $p; }

<<__EntryPoint>>
function test()[rx] {
  C::$p = 1;
}
