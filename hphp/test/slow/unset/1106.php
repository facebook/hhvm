<?hh

class A {
  static $foo = varray[123];
}

<<__EntryPoint>>
function main_1106() {
  $a = 'A';
  unset($a::$foo[0]);
  unset(A::$foo[0]);
}
