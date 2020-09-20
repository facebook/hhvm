<?hh

class MyClass {
  protected static function mapper(int $x): int {
    return $x + 3;
  }

  public function foo(Vector<int> $v): Vector<int> {
    return $v->map('MyClass::mapper');
  }
}

<<__EntryPoint>>
function main(): void {
  $c = new MyClass();
  var_dump($c->foo(Vector { 0, 1, 2, 3, 4, 5, 6 }));
}
