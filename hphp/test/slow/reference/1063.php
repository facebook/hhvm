<?hh

function run(inout $a, inout $b) :mixed{
  $c = $b;
  $b = 2;
  var_dump($a);
  var_dump($c);
}

<<__EntryPoint>>
function main_1063() :mixed{
  $a = 1;
  run(inout $a, inout $a);
}
