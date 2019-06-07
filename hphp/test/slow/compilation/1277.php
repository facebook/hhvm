<?hh

class A {
 const C = 123;
 static function t($a = B::C) {
}
 }
class B {
 const C = 456;
 static function t($a = A::C) {
}
 }

 <<__EntryPoint>>
function main_1277() {
A::t();
 B::t();
}
