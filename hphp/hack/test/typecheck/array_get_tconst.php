<?hh

abstract class C {
  abstract const type MyArray as array<mixed, mixed>;

  protected function f(this::MyArray $a): this::MyArray {
    $a['a'] = 4;
    return $a;
  }
}

final class D extends C {
  const type MyArray = array<string, string>;

  public function test(this::MyArray $a): this::MyArray {
    return $this->f($a);
  }
}
