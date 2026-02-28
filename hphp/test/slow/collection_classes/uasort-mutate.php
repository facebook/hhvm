<?hh

function nonstatic_string(string $a): string {
  return __hhvm_intrinsics\launder_value($a) . $a;
}

class Ref { public function __construct(public $v)[] {} }

<<__EntryPoint>>
function main() :mixed{
  $m = Map{};
  $m[1] = nonstatic_string('c');
  $m[2] = nonstatic_string('b');
  $m[3] = nonstatic_string('a');
  $mut = new Ref(true);
  uasort(inout $m, ($a, $b) ==> {
    var_dump($a, $b);
    if ($mut->v) {
      echo "mut\n";
      $mut->v = false;
      $m[0] = 'dd';
      unset($m[3]);
      unset($m[2]);
      unset($m[1]);
      unset($m[0]);
    }
    return $a <=> $b;
  });
  var_dump($m);
}
