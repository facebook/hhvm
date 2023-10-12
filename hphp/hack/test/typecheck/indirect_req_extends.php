<?hh // strict

class Base {
  public function foo(): ?int {
    return null;
  }
}

class Child extends Base {
  public function foo(): int {
    return 0;
  }
}

trait T1 {
  require extends Base;
}

class Grandchild extends Child {
  use T1;
}

trait T2 {
  require extends Grandchild;

  public function test(): void {
    // Ensure that T2 inherits Child::foo rather than Base::foo
    takes_int($this->foo());
  }
}

function takes_int(int $_): void {}
