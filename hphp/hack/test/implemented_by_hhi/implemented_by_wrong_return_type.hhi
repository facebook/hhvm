<?hh
function my_implementation(C $c): int;

final class C {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(): string;
}
