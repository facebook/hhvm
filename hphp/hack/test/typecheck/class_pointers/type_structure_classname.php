<?hh

class Bar {
  public static function foo(): void {}
}

class Foo {
  const type T = Bar;
}

function test_class_pointer(): void {
  $c = HH\type_structure_class(Foo::class, 'T');
  $c::foo(); // OK: class<Bar> allows static method calls
}

function test_classname(): void {
  $cn = HH\type_structure_classname(Foo::class, 'T');
  $cn::foo(); // ERROR: classname<Bar> does not allow static method calls
}
