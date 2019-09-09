<?hh

class C {
  const int X = 3 as ~int;
  public int $y = 4 as ~int;
  public static int $z = 5 as ~int;

  public function f(int $w = 6 as ~int): void {
    var_dump($w);
  }
}

const int D = 7 as ~int;

function f(int $i = 8 as ~int): void {
  var_dump($i);
  $j = 9 as ~int;
  var_dump($j);
}

<<__EntryPoint>>
function main(): void {
  var_dump(C::X);
  $c = new C();
  var_dump($c->y);
  var_dump(C::$z);
  $c->f();
  var_dump(D);
  f();
}
