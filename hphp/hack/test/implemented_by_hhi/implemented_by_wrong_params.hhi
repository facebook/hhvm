<?hh
function my_implementation(C $c, string $x): int;

class C {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(int $x): int;
}
