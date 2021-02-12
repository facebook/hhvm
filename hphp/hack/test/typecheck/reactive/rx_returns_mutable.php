<?hh // strict
class A {
  public function __construct(public int $a) {
  }
}


function Get(int $a): A {
  return new A($a);
}

function f(varray<int> $list): void {
  $b = array_map(fun('Get'), $list);
}
