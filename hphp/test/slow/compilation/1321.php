<?hh

class X {
  static function bar(X $x) :mixed{
    $x->foo();
    $x->foo();
  }
  function foo() :mixed{
 var_dump(__METHOD__);
 }
}
function test() :mixed{
  X::bar(null);
}

<<__EntryPoint>>
function main_1321() :mixed{
test();
}
