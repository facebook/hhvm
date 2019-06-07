<?hh

class C implements Countable {
 function count() {
 return 0;
 }
 }

<<__EntryPoint>>
function main_1867() {
var_dump(class_exists('C'));
}
