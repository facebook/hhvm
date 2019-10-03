<?hh

function run(inout $a, inout $b) {
  $a = 1;
  $c = $b;
  $a = 2;
  var_dump($b);
  var_dump($c);
}

<<__EntryPoint>>
function main() {
  $a = null;
  run(inout $a, inout $a);
}
