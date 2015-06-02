<?hh // strict

class Superclass {}
class Subclass extends Superclass {}

class :foo {
  attribute Superclass mysuperclass, Subclass mysubclass;
}

function main(): void {
  $x = <foo mysuperclass={new Superclass()} mysubclass={new Subclass()} />;
  $y = <foo mysuperclass={new Subclass()} />;
}
