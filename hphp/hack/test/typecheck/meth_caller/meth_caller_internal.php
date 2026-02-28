//// module_A.php
<?hh
new module A {}

//// C.php
<?hh

module A;

class C {
  internal function foo(): void { echo "I am foo\n"; }

  public function bar(): void {
    $f = meth_caller(C::class, 'foo');
    $c = new C();
    $f($c);
  }

  protected internal function baz(): void { echo "I am baz\n"; }

  public function qux(): void {
    $f = meth_caller(C::class, 'baz');
    $c = new C();
    $f($c);
  }
}
