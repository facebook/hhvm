<?hh

function foo($a, $b, $c) { var_dump($a, $b, $c); }

class X {
  const FOO=varray[1,2,3];
  static function bar() {
    foo(...self::FOO);
  }
}

<<__EntryPoint>>
function main_optimized_unpack() {
X::bar();
}
