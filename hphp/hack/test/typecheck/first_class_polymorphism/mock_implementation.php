<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

class TypedMockFunction<Tfun> {
  public function __construct(
    private HH\FunctionRef<Tfun> $functionRef,
  ): void {}

  <<__NoAutoLikes>>
  public function mockImplementation(
    <<__IgnoreReadonlyError>> Tfun $callable,
  ): this {
    throw new Exception();
  }
}

class Bigly {}
class Smol extends Bigly {}

class B<T as Bigly super Smol> {}

function top_level<T as Bigly super Smol>(B<T> $_): void {}

function mock_top_level(): void {
  $fptr = top_level<>;
  $impl = readonly function<T as Bigly super Smol>(B<T> $_): void {};
  $mock = new TypedMockFunction($fptr);
  $mock->mockImplementation($impl);
}
