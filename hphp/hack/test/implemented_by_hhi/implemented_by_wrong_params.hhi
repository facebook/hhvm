<?hh
function my_implementation(C $c, string $x): int;

final class C {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(int $x): int;
}
