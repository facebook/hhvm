<?hh

class Superclass {}
class Subclass extends Superclass {}

class :foo extends XHPTest {
  attribute
    Superclass mysuperclass,
    Subclass mysubclass;
}

function main(): void {
  $x = <foo mysuperclass={new Superclass()} mysubclass={new Subclass()} />;
  $y = <foo mysuperclass={new Subclass()} />;
}
