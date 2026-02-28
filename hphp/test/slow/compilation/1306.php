<?hh

function bar($a) :mixed{
}
function foo($x) :mixed{
  $a = $x;
  echo $x;
  unset($a);
  $a = bar(1);
  bar($a++);
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
