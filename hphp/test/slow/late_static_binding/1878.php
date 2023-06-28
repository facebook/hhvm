<?hh

class X {
  static function foo() :mixed{
 return false;
 }
  static function bar() :mixed{
 return 5.5;
 }
  static function baz() :mixed{
 return time();
 }
  }

<<__EntryPoint>>
function main_1878() :mixed{
var_dump(X::foo());
var_dump(X::bar());
var_dump(gettype(X::baz()));
}
