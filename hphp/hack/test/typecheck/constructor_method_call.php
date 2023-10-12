<?hh // strict

class A {
  public function foo(): void {
  }
}

function main(): void {
  new A()->foo();
}
