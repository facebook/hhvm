<?hh

interface IFoo {}

interface IBar {}

interface IQux extends IFoo, IBar {}

// Traits
trait Trait_1 {

}
trait Trait_2 {
  use Trait_1;
}

trait Trait_3 {
  use Trait_2;
}

// Classes
class Foo implements IFoo {}

class Bar extends Foo implements IQux {}

abstract class Baz extends Bar {
  use Trait_3;
}

function foo(): void {
  $baz = new Baz();
  //          ^ type-hierarchy-at-caret
}
