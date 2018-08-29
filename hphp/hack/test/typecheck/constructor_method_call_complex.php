<?hh // strict

class A {
  public function foo(): void {
  }
}

function main(?A $a): void {
  $a ?? new A() ?->foo();
}
