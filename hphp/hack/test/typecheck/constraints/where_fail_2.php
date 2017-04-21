<?hh // strict

class Base {
}

class Derived extends Base {
}

class FailWhere {
  public function assertSuperBase<T>(T $x): void where T super Base {
  }

  public function test(Derived $x): void {
    //TODO (t13262460): This test should actually fail, but doesn't, because of
    //a typehole in super-bounds. When the given task is finished, the .exp
    //should be updated with the appropriate error message
    $this->assertSuperBase($x);
  }
}
