<?hh

class C {
  public function foo(): this {
    return $this;
  }
}

class D extends C {}

function expectD(D $_): void {}

function testit(): void {
  $d = new D();
  // Accepted
  expectD($d->foo());
  $f = meth_caller(C::class, 'foo');
  // Currently rejected, but could be made safe
  // if $f were typed with generic function type (function<Tthis as C>(Tthis):Tthis)
  expectD(($f)($d));
}
