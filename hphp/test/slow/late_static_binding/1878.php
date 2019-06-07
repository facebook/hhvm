<?hh

class X {
  static function foo() {
 return false;
 }
  static function bar() {
 return 5.5;
 }
  static function baz() {
 return time();
 }
  }

<<__EntryPoint>>
function main_1878() {
var_dump(X::foo());
var_dump(X::bar());
var_dump(gettype(X::baz()));
}
