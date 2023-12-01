<?hh

class C {
  public static function f(): void {
    nameof meth_caller("Foo", "bar");
  }
}
