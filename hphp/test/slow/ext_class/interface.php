<?hh

interface A {function a();}
interface B extends A {}

<<__EntryPoint>>
function main_interface() {
var_dump(method_exists('B', 'a'));
}
