<?hh

function val() { return 0; }
function foo($k) {
  $array = varray[0, 1];
  $idx = val();
  for ($ik = 0; $ik < 10; ++$ik) {
  }

  $idx++;
  echo $array[$idx];
  echo "\n";
}
<<__EntryPoint>> function main(): void {
foo(12);
}
