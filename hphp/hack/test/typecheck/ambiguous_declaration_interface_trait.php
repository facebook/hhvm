<?hh // partial
interface I1 {
  public function foo(): num;
}
trait T {
  abstract public function foo(): int;
}

class C implements I1 {
  use T;
}
