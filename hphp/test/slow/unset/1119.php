<?hh

function run(inout $a, inout $c) {
  $b = array(0, 1);
  $a = array($b, 1);
  unset($a[0][0]);
  var_dump($a);
}

<<__EntryPoint>>
function main() {
  $a = null;
  run(inout $a, inout $a);
}
