<?hh

class MyClass {
  protected static function mapper(int $x): int {
    return $x + 3;
  }

  public function foo(Vector<int> $v): void {
    return $v->map('MyClass::mapper');
  }
}

function main(): void {
  $c = new MyClass();
  var_dump($c->foo(Vector { 0, 1, 2, 3, 4, 5, 6 }));
}

try {
  main();
} catch (Exception $e) { echo "nodice\n"; }
