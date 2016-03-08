<?hh

class FooCls {
  function __construct($a, $b, $c, $d) {
    $this->what = array($a, $b, $c, $d);
  }
  function why() { return $this->what; }
}

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

$small = array('array_fill', 0, 10, 'foo');
$large = array('array_fill', 0, 10000000000, 'foo');
$easy = array($small, $small, $small, $small);
$hard = array($small, $small, $small, $small, $large);

__hhvm_intrinsics\disable_inlining('foo_maker');
__hhvm_intrinsics\disable_inlining('FooCls::__construct');
__hhvm_intrinsics\disable_inlining('FooCls::why');

main(false, $small);
main(false, $small);
main(false, $small);
main(true, $large);
