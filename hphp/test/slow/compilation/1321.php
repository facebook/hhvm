<?hh

class X {
  static function bar(X $x) {
    $x->foo();
    $x->foo();
  }
  function foo() {
 var_dump(__METHOD__);
 }
}
function test() {
  X::bar(null);
}

<<__EntryPoint>>
function main_1321() {
test();
}
