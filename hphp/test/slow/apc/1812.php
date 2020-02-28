<?hh

function foo($ref, $a) {
  $ref->a = $a;
  print_r($a);
  apc_store('table', $a);
}

class Ref { public $a; }

<<__EntryPoint>>
function main() {
  $aa = new Ref();
  $a = varray[$aa];
  foo($aa, $a);
}
