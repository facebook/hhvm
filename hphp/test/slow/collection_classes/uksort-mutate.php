<?hh

function nonstatic_string(string $a): string {
  return __hhvm_intrinsics\launder_value($a) . $a;
}

class Ref { public function __construct(public $v)[] {} }

<<__EntryPoint>>
function main() :mixed{
  $m = Map{};
  $m[nonstatic_string('c')] = 1;
  $m[nonstatic_string('b')] = 2;
  $m[nonstatic_string('a')] = 3;
  $mut = new Ref(true);
  uksort(inout $m, ($a, $b) ==> {
    var_dump($a, $b);
    if ($mut->v) {
      echo "mut\n";
      $mut->v = false;
      $m['dd'] = 0;
      unset($m['aa']);
      unset($m['bb']);
      unset($m['cc']);
      unset($m['dd']);
    }
    return $a <=> $b;
  });
  var_dump($m);
}
