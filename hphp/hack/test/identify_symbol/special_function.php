<?hh

class C {
  public function foo() : void {
    meth_caller(C::class, "foo");
  }
}
