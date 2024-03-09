<?hh

class Foo {
  public function foo(): void {
    $f = ($this) ==> $this->foo();
    $f2 = (...$this) ==> $this->foo();
    $f3 = $this ==> $this->foo();
  }
}
