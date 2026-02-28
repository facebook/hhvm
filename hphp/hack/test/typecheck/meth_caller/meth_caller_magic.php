<?hh

class Foo {
  public function __construct() {}
}

function test(): void {
  HH\meth_caller(Foo::class, '__construct');
}
