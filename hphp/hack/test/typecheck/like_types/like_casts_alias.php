<?hh

class C {
  const int X = "3" as HH\INCORRECT_TYPE<int>;
  public int $y = "4" as HH\INCORRECT_TYPE<int>;
  public static int $z = "5" as HH\INCORRECT_TYPE<int>;

  public function f(int $w = "6" as HH\INCORRECT_TYPE<int>): void {}
}

const int D = "7" as HH\INCORRECT_TYPE<int>;

function f(int $i = "8" as HH\INCORRECT_TYPE<int>): void {
  $j = "9" as HH\INCORRECT_TYPE<int>;
}
