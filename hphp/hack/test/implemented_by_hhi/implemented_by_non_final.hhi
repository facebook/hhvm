<?hh
function my_implementation(C $c): int;
function my_implementationI(I $c): int;

class C {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(): int;
}

interface I {
  <<__ImplementedBy('\my_implementationI')>>
  public function myMethod(): int;
}
