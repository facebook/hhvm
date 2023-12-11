<?hh

class Ref {
  public function __construct(public $val) {}
}

function new_closure_gen() :mixed{
  $ref = new Ref(0);
  return function() use ($ref) {
    yield ++$ref->val;
  };
}
<<__EntryPoint>> function main(): void {
$closure1 = new_closure_gen();
$closure2 = new_closure_gen();

$gen1 = $closure1();
$gen2 = $closure1();
$gen3 = $closure2();

foreach (vec[$gen1, $gen2, $gen3] as $gen) {
    foreach ($gen as $val) {
        var_dump($val);
    }
}
}
