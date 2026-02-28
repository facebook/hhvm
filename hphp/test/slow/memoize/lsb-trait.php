<?hh

trait T {
  <<__Memoize>>
  public static function foo(?string $c = null): this {
    return new static();
  }
}

abstract class A { use T; }
class B extends A {}
class C extends A {}

<<__EntryPoint>>
function main(): void {
  $x = __hhvm_intrinsics\launder_value('B');
  var_dump(get_class($x::foo()));
  var_dump(get_class($x::foo()));
}
