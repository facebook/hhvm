<?hh

function run(inout $a, inout $b) :mixed{
  $a = 1;
  $c = $b;
  $a = 2;
  var_dump($b);
  var_dump($c);
}

<<__EntryPoint>>
function main() :mixed{
  $a = null;
  run(inout $a, inout $a);
}
