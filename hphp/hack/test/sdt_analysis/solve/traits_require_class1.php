<?hh

<<file:__EnableUnstableFeatures('require_class')>>

abstract class Abs {
  public abstract function foo(vec<int> $v): void;
}

trait Tr {
  require class C;
}

final class C extends Abs {
  use Tr;
  public function foo(vec<int> $v): void {}
}

function foo(Abs $i, dynamic $dyn): void {
  $i->foo($dyn);
}
