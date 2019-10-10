<?hh // strict
class A {
  public function m1(): string {
    return "a";
  }
}

trait Trait1 {
  function m1(): string {
    return "1";
  }
}

class B extends A {
  use Trait1;
}

class C extends A {
  use Trait1;
}
