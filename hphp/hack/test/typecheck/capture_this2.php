<?hh // partial

class A {
  public function g(): void {
    $a = function() use ($this) {};
  }
}
