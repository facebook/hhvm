<?hh

abstract class Base {
  private function f(): int { return 0; }
}

trait T {
  require extends Base;
}

class Child extends Base {
  use T;
  public function g(): int { return $this->f(); }
}
