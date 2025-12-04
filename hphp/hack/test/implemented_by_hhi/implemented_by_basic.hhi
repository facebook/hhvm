<?hh
function my_implementation(C $c): int;

class C {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(): int;
}
