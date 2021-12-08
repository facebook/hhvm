<?hh

class C { public static $p; }

function test()[rx, globals] {
  C::$p['a'] = 'b';
}

<<__EntryPoint>>
function main() {
  C::$p = dict[];
  test();
}
