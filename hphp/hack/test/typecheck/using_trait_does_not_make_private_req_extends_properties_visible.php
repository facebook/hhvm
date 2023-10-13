<?hh

abstract class Base {
  private int $x = 0;
}

trait T {
  require extends Base;
}

class Child extends Base {
  use T;
  public function f(): int { return $this->x; }
}
