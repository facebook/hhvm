<?hh

abstract class Abs {
  public abstract function foo(vec<int> $v): void;
}

trait Tr1 {
  require extends Abs;
  public function foo(vec<int> $_): void {}
}

final class C extends Abs {
  use Tr1;
}

function foo(Abs $i, dynamic $dyn): void {
  $i->foo($dyn);
}
