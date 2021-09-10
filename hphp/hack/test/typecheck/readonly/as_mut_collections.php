<?hh
<<file:__EnableUnstableFeatures("readonly")>>

class Foo {
  public function __construct(public readonly (readonly function(): void) $prop) {}
}

function test(readonly vec<int> $x) : void {
  HH\Readonly\as_mut($x);
}

function test_multiple(readonly dict<int, vec<int>> $x) : void {
  HH\Readonly\as_mut($x);
}

function test_generic<T as vec<int>>(readonly T $y): void {
  HH\Readonly\as_mut($y);
}

function test_vec_objects(readonly vec<Foo> $y) : void {
  HH\Readonly\as_mut($y); // error
}

function test_shape(readonly shape("a" => int) $y) : void {
  HH\Readonly\as_mut($y);
}
