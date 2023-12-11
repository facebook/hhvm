<?hh

class FooCls {
  <<__NEVER_INLINE>>
  function __construct($a, $b, $c, $d) {
    $this->what = vec[$a, $b, $c, $d];
  }
  <<__NEVER_INLINE>>
  function why() :mixed{ return $this->what; }
}

<<__NEVER_INLINE>>
function foo_maker(bool $b, $x) :mixed{
  __hhvm_intrinsics\trigger_oom($b);
  $y = new FooCls(...$x);
  return $y->why();
}

function foo_maker_caller(bool $b, varray $x) :mixed{
  $t = foo_maker($b, $x);
  return $t;
}

function main(bool $b, varray $x) :mixed{
  $q = foo_maker_caller($b, $x);
  return count($q);
}


<<__EntryPoint>>
function main_call_array_crash() :mixed{
$small = vec['array_fill', 0, 10, 'foo'];
$large = vec['array_fill', 0, 10000000000, 'foo'];
$easy = vec[$small, $small, $small, $small];
$hard = vec[$small, $small, $small, $small, $large];

main(false, $small);
main(false, $small);
main(false, $small);
main(true, $large);
}
