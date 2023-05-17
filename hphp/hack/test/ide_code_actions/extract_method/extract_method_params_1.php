<?hh

class Foo<T> {
  public function __construct(public T $x) {}
}

class Klass {
  public function foo(): void {
    $ignore1 = 1;
    $param1 = 1;
    $param2 = "";
    $param2 = new Foo($param2);
    /*range-start*/
    $param1 = $param1;
    $z = $param2;
    /*range-end*/
    $ignore2 = 1;
  }
}
