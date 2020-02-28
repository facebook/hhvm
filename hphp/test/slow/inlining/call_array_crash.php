<?hh

class FooCls {
  <<__NEVER_INLINE>>
  function __construct($a, $b, $c, $d) {
    $this->what = varray[$a, $b, $c, $d];
  }
  <<__NEVER_INLINE>>
  function why() { return $this->what; }
}

<<__NEVER_INLINE>>
function foo_maker(bool $b, $x) {
  __hhvm_intrinsics\trigger_oom($b);
  $y = new FooCls(...$x);
  return $y->why();
}

function foo_maker_caller(bool $b, array $x) {
  $t = foo_maker($b, $x);
  return $t;
}

function main(bool $b, array $x) {
  $q = foo_maker_caller($b, $x);
  return count($q);
}


<<__EntryPoint>>
function main_call_array_crash() {
$small = varray['array_fill', 0, 10, 'foo'];
$large = varray['array_fill', 0, 10000000000, 'foo'];
$easy = varray[$small, $small, $small, $small];
$hard = varray[$small, $small, $small, $small, $large];

main(false, $small);
main(false, $small);
main(false, $small);
main(true, $large);
}
