<?hh

class Box<T> {}

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

  public function mockReturn<Tr>(Tr $v): this
  where
    Tfun super (readonly function(mixed...)[]: Tr) {
    return $this->mockImplementation((mixed ...$_args)[]: Tr ==> $v);
  }
}

function blah<T>(Box<T> $_): bool {
  return true;
}

function mock_it(): void {
  $ref = blah<>;
  $mock = new TypedMockFunction($ref);
  $mock->mockReturn(false);
}
