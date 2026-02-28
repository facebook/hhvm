<?hh

class C implements Countable {
 function count() :mixed{
 return 0;
 }
 }

<<__EntryPoint>>
function main_1867() :mixed{
var_dump(class_exists('C'));
}
