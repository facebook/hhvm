<?hh

class X {
  static function g() :mixed{
}
}
function g($a,$b) :mixed{
}
function f() :mixed{
 return 3;
 }

<<__EntryPoint>>
function main_1294() :mixed{
;
@X::g();
@g(f(),f());
}
