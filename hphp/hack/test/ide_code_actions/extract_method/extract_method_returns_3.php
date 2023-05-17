<?hh

class Foo<T> {
  public function __construct(public T $x) {}
}

class Klass {
  public function foo(): void {
    $ignore1 = 1;
    /*range-start*/
    $return1 = 100;
    $local = 500 + $return1;
    $return2 = new Foo($local);
    /*range-end*/
    $ignore2 = 1 + $return1;
    var_dump($return2);
  }
}
