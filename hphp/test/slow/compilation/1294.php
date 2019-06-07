<?hh

class X {
  static function g() {
}
}
function g($a,$b) {
}
function f() {
 return 3;
 }

<<__EntryPoint>>
function main_1294() {
;
@X::g();
@g(f(),f());
}
