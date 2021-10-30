<?hh
<<file:__EnableUnstableFeatures("readonly")>>

class Foo {
  public function __construct(public readonly (readonly function(): void) $prop) {}
}

enum Bar : int as int {
  Top = 0;
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

function test_null() : void {
  HH\Readonly\as_mut(readonly null); // ok
}

function test_nullable(readonly ?Foo $y) : void {
  HH\Readonly\as_mut($y); // error
}

function test_classname(readonly ?classname<Foo> $x) : void {
  HH\Readonly\as_mut($x); // ok
}

function test_enum(readonly ?Bar $x): void {
  HH\Readonly\as_mut($x); // ok
}
