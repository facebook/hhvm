<?hh
interface Rx {

  public function foo(): this;
}

abstract class MyParent {

  public function foo(): this {
    return $this;
  }
}

class MyChild extends MyParent {
  public function foo(): this {
    return $this;
  }
}
