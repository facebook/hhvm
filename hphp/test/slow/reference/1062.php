<?hh

function run(inout $a, inout $b) :mixed{
  $b = 2;
  var_dump($a);
}

<<__EntryPoint>>
function main() :mixed{
  $a = 1;
  run(inout $a, inout $a);
}
