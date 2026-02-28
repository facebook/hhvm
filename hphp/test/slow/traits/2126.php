<?hh

trait T {
 public function y() :mixed{
}
 }
interface I {
 public function y():mixed;
}
class C {
 use T ;
 }

<<__EntryPoint>>
function main_2126() :mixed{
var_dump(method_exists('T', 'y'));
var_dump(method_exists('C', 'y'));
var_dump(method_exists('I', 'y'));
var_dump(method_exists('T', 'x'));
var_dump(method_exists('C', 'x'));
var_dump(method_exists('I', 'x'));
}
