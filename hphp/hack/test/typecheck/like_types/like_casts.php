<?hh

class C {
  const int X = "3" as ~int;
  public int $y = "4" as ~int;
  public static int $z = "5" as ~int;

  public function f(int $w = "6" as ~int): void {}
}

const int D = "7" as ~int;

function f(int $i = "8" as ~int): void {
  $j = "9" as ~int;
}
