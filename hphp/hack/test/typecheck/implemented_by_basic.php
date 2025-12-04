<?hh
function my_implementation(C $c): int { return 0; }

class C {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(): int {
    return 0;
  }
}
