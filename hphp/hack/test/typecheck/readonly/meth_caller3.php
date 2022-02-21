<?hh

class B {
  public readonly function this_readonly(): void {}
}

function test(): void {
  $x = meth_caller(B::class, 'this_readonly');
}
