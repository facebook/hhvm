<?hh // strict

class Superclass {}
class Subclass extends Superclass {}

class :foo {
  attribute Superclass mysuperclass, Subclass mysubclass;
}

function main(): void {
  // type error
  $x = <foo mysubclass={new Superclass()} />;
}
