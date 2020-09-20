<?hh // strict

interface I1<T> {
  public function dupe1(int $x): void;
  public function dupe2(): vec<string>;
  public function dupe3(): T;
}

interface I2<T> {
  public function dupe1(int $x): void;
  public function dupe2(): vec<string>;
  public function dupe3(): T;
}

function test<T as I1<int> as I2<int>>(T $x): void {
  $x->dupe1(0);
  $x->dupe2();
  $x->dupe3();
}
