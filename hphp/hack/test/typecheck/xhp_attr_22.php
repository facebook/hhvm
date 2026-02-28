<?hh

class Superclass {}
class Subclass extends Superclass {}

class :foo extends XHPTest {
  attribute
    Superclass mysuperclass,
    Subclass mysubclass;
}

function main(): void {
  // type error
  $x = <foo mysubclass={new Superclass()} />;
}
