<?hh

function foo() {
  $bar = $GLOBALS['asd'];
}
<<__EntryPoint>> function main(): void {
// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

foo();
$x = darray(HH\global_keys());
sort(inout $x);
foreach ($x as $k) { echo "$k->".$GLOBALS[$k]."\n"; }
}
