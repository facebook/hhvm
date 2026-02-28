<?hh

function run(inout $a, inout $c) :mixed{
  $b = vec[0, 1];
  $a = vec[$b, 1];
  unset($a[0][1]);
  var_dump($a);
}

<<__EntryPoint>>
function main() :mixed{
  $a = null;
  run(inout $a, inout $a);
}
