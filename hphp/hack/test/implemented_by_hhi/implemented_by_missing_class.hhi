<?hh
function my_implementation(string $s): int;

final class C {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(string $s): int;
}
