<?hh

function run(inout $a, inout $b, inout $c, inout $d) {
  $b = $d;
  var_dump($a);
  var_dump($b);
  var_dump($c);
  var_dump($d);
}

<<__EntryPoint>>
function main() {
  $a = 1;
  $c = 2;
  run(inout $a, inout $a, inout $c, inout $c);
}
