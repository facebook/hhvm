<?hh
// In a final class `this` is nominally the class itself, so there is nothing to
// abstract and pointers are always sound, however `this` appears.
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

final class Box<T> {}

final class F {
  const type T = int;
  public static function two_bare(this $_, this $_): void {}
  public static function two_box(Box<this> $_, Box<this> $_): void {}
  public static function returns_two(
    (function(): this) $_,
    (function(): this) $_,
  ): void {}
  public function one_bare(this $_): void {}
}

function test(): void {
  $ok1 = F::two_bare<>;
  $ok2 = F::two_box<>;
  $ok3 = F::returns_two<>;
  $ok4 = meth_caller(F::class, 'one_bare');
}
