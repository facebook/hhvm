<?hh

function run(inout $a, inout $c) {
  $b = varray[0, 1];
  $a = varray[$b, 1];
  unset($a[0][0]);
  var_dump($a);
}

<<__EntryPoint>>
function main() {
  $a = null;
  run(inout $a, inout $a);
}
