<?hh
interface Rx {

  public function foo(): this;
}

abstract class MyParent {
  // OK since it cannot be called in non-rx code
  public function foo(): this {
    return $this;
  }
}

class MyChild extends MyParent {

  public function foo(): this {
    return $this;
  }
}
