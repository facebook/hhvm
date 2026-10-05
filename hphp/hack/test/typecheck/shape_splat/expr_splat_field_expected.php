<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Fields that cannot be replaced by a later splat receive their expected type.

class C {
  public function foo(): int {
    return 1;
  }
}

type Base = shape('a' => int);

function want(shape('a' => int, 'cb' => (function(C): int)) $s): void {}

function with_splat(Base $base): void {
  want(shape(...$base, 'cb' => $x ==> $x->foo()));
}

function without_splat(): void {
  want(shape('a' => 1, 'cb' => $x ==> $x->foo()));
}

function want_polymorphic<T as shape(...)>(
  shape(...T, 'cb' => (function(C): int)) $s,
): void {}

function with_polymorphic_expected<T as shape(...)>(T $base): void {
  want_polymorphic<T>(shape(...$base, 'cb' => $x ==> $x->foo()));
}

function discarded_string(): string {
  return "discarded";
}

function masked_field(
  shape('x' => int) $tail,
): shape('x' => int) {
  return shape('x' => discarded_string(), ...$tail);
}
