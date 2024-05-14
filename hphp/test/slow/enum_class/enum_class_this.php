<?hh

final class Foo {
  public function __construct((function()[]: void) $call)[] {}
}
enum class Foos: Foo {
  // Error: $this not defined in enum classes
  Foo EXAMPLE = $this;
}
