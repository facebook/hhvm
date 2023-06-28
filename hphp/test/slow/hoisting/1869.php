<?hh

trait t {
 }
class C {
 use t;
 }

<<__EntryPoint>>
function main_1869() :mixed{
var_dump(class_exists('C'));
}
