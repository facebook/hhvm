<?hh

class C { public function __construct(public $x)[] {} }

function test($a, C $b) :mixed{
  $xs = vec[];
  if ($a) {
    $xs = vec[$a];
  }
  // We trigger the bug here, since $xs == TVArr|TVec == TArrLike.
  // Previously, we returned TBottom as the new base type here.
  $xs[] = $b;

  foreach ($xs as $x) {
    var_dump($x->x);
  }
}

<<__EntryPoint>>
function main(): void {
  test(new C(17), new C(34));
}
