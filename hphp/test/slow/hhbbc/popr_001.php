<?hh

class A {}

function bar(inout $a) :mixed{
  return $a[0];
}

function main() :mixed{
  $a = vec[new A];
  bar(inout $a);
}


<<__EntryPoint>>
function main_popr_001() :mixed{
main();
}
