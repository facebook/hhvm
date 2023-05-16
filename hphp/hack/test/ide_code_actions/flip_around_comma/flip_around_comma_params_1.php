<?hh

class Box<T> {
  public function __construct(public T $val){}
}

class Klass {
  public function foo(Box<int> $a, /*range-start*//*range-end*/int $b): void {}
}
