<?hh

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

function blah<Tblah super bool>(int $_): Tblah {
  throw new Exception();
}

function mock_it(): void {
  $fptr = blah<>;

  $mock = new TypedMockFunction($fptr);

  $mock->mockImplementation((int $_) ==> false);
}
