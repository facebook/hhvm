<?hh

function takes_string(string $s): void {}

function f(string ...$args): void {
  foreach ($args as $arg) {
    takes_string($arg);
  }
}

class C1 {
  public function meth(...$args): void {}
}
