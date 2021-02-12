<?hh
class A {
  public ?int $v;
}


function f(dict<int, A> $a)[]: void {
  // OK
  unset($a[0]);
}
