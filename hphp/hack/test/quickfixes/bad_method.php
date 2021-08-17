<?hh

class Foo {
  public function instanceMeth(): void {}
  public static function staticMeth(int $_): void {}
}

function takes_foo(Foo $f): void {
  $f->instanceMethOops();

  Foo::staticMethOops("stuff");
}
