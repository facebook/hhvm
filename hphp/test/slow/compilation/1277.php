<?hh

class A {
 const C = 123;
 static function t($a = B::C) :mixed{
}
 }
class B {
 const C = 456;
 static function t($a = A::C) :mixed{
}
 }

 <<__EntryPoint>>
function main_1277() :mixed{
A::t();
 B::t();
}
