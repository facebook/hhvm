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

function bar_1(Super $f): void {
  $g = $f;
  foo($g);
}

function bar_2(Super $f): void {
  $g = $f;
  $h = $g;
  foo($h);
}

function bar_3(Super $f): void {
  $g = $f;
  $h = $g;
  $i = $h;
  foo($i);
}

function bar_4(SuperBox $f): void {
  $g = $f;
  $h = $g->get();
  $i = $h;
  foo($i);
}
