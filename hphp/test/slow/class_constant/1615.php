<?hh

class B {
 const X = 'old';
 }
class A extends B {
 const X = 'new';
 }

<<__EntryPoint>>
function main_1615() :mixed{
var_dump(A::X);
var_dump(get_class_constants('A'));
}
