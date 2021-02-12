<?hh // partial
class A {
  public ?int $v;
}


function f(A $a)[]: void {
  // ERROR
  unset($a->v);
}
