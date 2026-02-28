<?hh

class ExpectObj<T> {
  final public function check(): void {}

  final public function toBeInt(): void
    where T = int {}

  final public function notToThrow<TRet>(): void
    where T = (function(): TRet) {}

  final public function __construct(protected T $obj) {}
}

function expect<T>(T $obj): ExpectObj<T> {
  return new ExpectObj<T>($obj);
}

function funValue(): (function(): void)  {
  $f = function() { return; };
  return $f;
}

function myTest(): void {
  expect(funValue())->AUTO332
}
