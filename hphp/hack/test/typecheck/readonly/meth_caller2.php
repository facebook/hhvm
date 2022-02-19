<?hh

class A {}

class B {
  public function takes_readonly(readonly A $_): void {}
}

function test(): void {
  $x = meth_caller(B::class, 'takes_readonly');
}
