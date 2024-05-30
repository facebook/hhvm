<?hh

class Super {}
class Sub extends Super {}
class SuperBox {
  public function __construct(private Super $item) {}
  public function get(): Super {
    return $this->item;
  }
}

function foo(Sub $_): void {}

function bar(SuperBox $f): void {
  $g = $f->get();
  foo($g);
}
