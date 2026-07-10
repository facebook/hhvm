<?hh

interface UpperOne {}
interface UpperTwo {}
class Lower implements UpperOne, UpperTwo {}
class LowerOne extends Lower {}
class LowerTwo extends Lower {}

abstract class Base {
  abstract const type T as UpperOne as UpperTwo super LowerOne super LowerTwo;

  public function take(this::T $_): void {}
}

class Impl extends Base {
  const type T = Lower;
}

function use_it(Impl $impl, LowerOne $lower_one, LowerTwo $lower_two): void {
  $impl->take($lower_one);
  $impl->take($lower_two);
}
