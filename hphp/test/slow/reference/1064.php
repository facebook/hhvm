<?hh

function run(inout $a, inout $b) {
  $c = $b;
  $b = 2;
  var_dump($a);
  var_dump($c);
}

<<__EntryPoint>>
function main() {
  $a = 1;
  run(inout $a, inout $a);
}
