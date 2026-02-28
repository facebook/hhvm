<?hh

class A {
 const A = 'a';
 }
class B extends A {
 const B = 'b';
 }

<<__EntryPoint>>
function main_1613() :mixed{
var_dump(get_class_constants('B'));
}
