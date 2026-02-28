<?hh

class X {
  static function foo() :mixed{
 echo "X::foo\n";
 }
  function bar() :mixed{
 static::foo();
 }
}
class Y extends X {
  static function foo() :mixed{
 echo "Y::foo\n";
 }
  function baz() :mixed{
 X::bar();
 }
}

<<__EntryPoint>>
function main_1879() :mixed{
$y = new Y;
$y->baz();
}
