<?hh

class B {
 const X='xxx';
 }
class C {
 const Y=B::X;
 }

<<__EntryPoint>>
function main_1617() :mixed{
var_dump(get_class_constants('C'));
}
