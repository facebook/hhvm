<?hh

function run(inout $a, inout $b) :mixed{
  $a = 10;
  var_dump($b);
}

<<__EntryPoint>>
function main() :mixed{
  $a = null;
  run(inout $a, inout $a);
}
