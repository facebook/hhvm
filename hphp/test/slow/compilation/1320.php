<?hh

class X {
  static function foo() :mixed{
 return new X;
 }
  function bar() :mixed{
 var_dump(__METHOD__);
 }
}
function id($x) :mixed{
 return $x;
 }
function test() :mixed{
  id(X::foo(1))->bar();
}

<<__EntryPoint>>
function main_1320() :mixed{
;
test();
}
