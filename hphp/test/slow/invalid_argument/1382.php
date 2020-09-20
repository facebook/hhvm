<?hh

function bar($a) {
 return $a;
 }
function baz($a) {
 return $a;
 }
function foo($x) {
  return call_user_func(fun('baz'), call_user_func(fun('bar'), $x));
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
