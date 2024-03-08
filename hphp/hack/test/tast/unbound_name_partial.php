<?hh

function test(): mixed {
  meth_caller(NoSuchClass::class, 'foo');
}
