<?hh

function bar($a) :mixed{
 return $a;
 }
function baz($a) :mixed{
 return $a;
 }
function foo($x) :mixed{
  return call_user_func(baz<>, call_user_func(bar<>, $x));
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
