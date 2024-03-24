<?hh

module A;

class C {
  internal function foo(): void { echo "I am foo\n"; }

  public function bar(): void {
    $f = meth_caller(C::class, 'foo');
    $c = new C();
    $f($c);
  }
}

<<__EntryPoint>>
function main(): void {
  (new C())->bar();
}
