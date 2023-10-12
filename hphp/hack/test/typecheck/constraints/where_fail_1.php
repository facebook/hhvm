<?hh // strict

class Base {
}

class Derived extends Base {
}

class FailWhere {
  public function assertAsDerived<T>(T $x): void where T as Derived {
  }

  public function test(Base $x): void {
    $this->assertAsDerived($x);
  }
}
