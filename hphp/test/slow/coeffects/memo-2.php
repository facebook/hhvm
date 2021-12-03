<?hh

class A {
  <<__PolicyShardedMemoize>>
  static function f()[zoned] {
    echo "ok: " . static::class ."\n";
  }
  <<__PolicyShardedMemoizeLSB>>
  static function f_lsb()[zoned] {
    echo "ok_lsb: " . static::class ."\n";
  }
}

class B extends A {}
class C extends B {}

<<__EntryPoint>>
function main() {
  $classes = vec['A', 'B', 'C'];

  foreach ($classes as $c) {
    $c::f();
    $c::f();
    $c::f_lsb();
    $c::f_lsb();
  }
}
