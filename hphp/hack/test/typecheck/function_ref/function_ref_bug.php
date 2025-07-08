<?hh

class MockFunction<Tfun> {
  public function __construct(public HH\FunctionRef<Tfun> $fun) {}
  public function mock(<<__IgnoreReadonlyError>> Tfun $mock): void {}
}

<<__NoAutoLikes>>
function makeMockFunction<Tfun>(HH\FunctionRef<Tfun> $fun): MockFunction<Tfun> {
  return new MockFunction($fun);
}

<<__EntryPoint>>
function testit(): void {
  $app_id = 3;
  $mf = new MockFunction(other<>);
  $mf->mock(($id)[] ==> $id !== $app_id);
}

function other(mixed $fbid)[globals, leak_safe]: bool {
  return false;
}
