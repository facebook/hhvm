<?hh

class C<reify TC> {
  public function foo<reify TFoo>(TFoo $x): void {}
}

function refIt(): void {
  $f = meth_caller(C::class, 'foo');
}
