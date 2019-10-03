<?hh

function run(inout $a, inout $b) {
  $b = 2;
  var_dump($a);
}

<<__EntryPoint>>
function main() {
  $a = 1;
  run(inout $a, inout $a);
}
