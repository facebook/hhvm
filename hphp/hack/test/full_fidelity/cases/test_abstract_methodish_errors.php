<?hh

interface I1 {
  public function i1(): void; // legal
  public abstract function i2(): void; // error2045
}

class C1 { // technically is half of errror2044, but we report error2044 later
  public function c2(): void { } // legal
  public abstract function c3(): void; // error2044 reported here
}

abstract class C2 { // legal
  public function c1(): void { } // legal
  public abstract function c2(): void; // legal
}

trait T {
  public function t1(): void { } // legal
  public abstract function t2(): void; // legal
}
