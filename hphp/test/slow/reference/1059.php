<?hh

function run(inout $a, inout $b) {
  $a = 10;
  var_dump($b);
}

<<__EntryPoint>>
function main() {
  $a = null;
  run(inout $a, inout $a);
}
