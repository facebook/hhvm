<?hh

interface IFoo {}

interface IBar {}

interface IQux extends IFoo, IBar {}

// Traits
trait Trait_1 {
  int $prop_from_Trait_1 = 0;

  static int $static_prop_from_Trait_1 = 1;

  function method_from_trait_1(): void {}

  static function static_method_from_Trait_1(): void {}
}

trait Trait_2 {
  use Trait_1;
}

trait Trait_3 {
  use Trait_2;
}

// Classes
class Foo implements IFoo {
  const int const_from_Foo = 2;

  int $prop_from_Foo = 0;

  static int $static_prop_from_Foo = 1;

  function method_from_Foo(): void {}

  static function static_method_from_Foo(): void {}
}

class Bar extends Foo implements IQux {}

abstract class Baz extends Bar {
  use Trait_3;

  const int const_from_Baz = 2;

  int $prop_from_Baz = 0;

  static int $static_prop_from_Baz = 1;

  function method_from_Baz(int $val, int ...$vals): void {}

  static function static_method_from_Baz(): void {}
}

function foo(): void {
  $baz = new Baz();
  //          ^ type-hierarchy-at-caret
}
