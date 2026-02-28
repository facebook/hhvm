<?hh

function foo($ref, $a) :mixed{
  $ref->a = $a;
  print_r($a);
  apc_store('table', $a);
}

class Ref { public $a; }

<<__EntryPoint>>
function main() :mixed{
  $aa = new Ref();
  $a = vec[$aa];
  foo($aa, $a);
}
