<?hh

function run(inout $a, inout $b) {
  $c = 2;
  $b = $c;
  $c = 5;
  var_dump($a);
  var_dump($b);
  var_dump($c);
}

<<__EntryPoint>>
function main() {
  $a = 1;
  run(inout $a, inout $a);
}
