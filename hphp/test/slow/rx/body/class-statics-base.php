<?hh

class C { public static $p; }

<<__Rx>>
function test() {
  C::$p['a'] = 'b';
}

<<__EntryPoint>>
function main() {
  C::$p = dict[];
  test();
}
