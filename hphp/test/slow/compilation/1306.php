<?hh

function bar($a) {
}
function foo($x) {
  $a = $x;
  echo $x;
  unset($a);
  $a = bar(1);
  bar($a++);
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
