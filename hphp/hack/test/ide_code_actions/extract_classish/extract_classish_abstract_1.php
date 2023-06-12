<?hh

// We don't provide "Extract interface" refactors for abstract classes.
// Probably nothing wrong with providing it, though, just needs more
// logic to handle abstract methods.
abstract class A {
  /*range-start*/
  public abstract function foo(): void;
  public function bar(): void {
    echo 1 + 1;
  }
  /*range-end*/
}
