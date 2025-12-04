<?hh
function my_implementation(C $c, int $x, int $y): int;

class C {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(int $x): int;
}
