<?hh

interface A {function a():mixed;}
interface B extends A {}

<<__EntryPoint>>
function main_interface() :mixed{
var_dump(method_exists('B', 'a'));
}
