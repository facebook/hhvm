<?hh

trait t {
 }
class C {
 use t;
 }

<<__EntryPoint>>
function main_1869() {
var_dump(class_exists('C'));
}
