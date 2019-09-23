<?hh

<<__Sealed(SomeTrait::class)>>
interface SomeSealedInteface {}

trait SomeTrait implements SomeSealedInteface {}

class SomeClass {
  use SomeTrait;
}

trait SomeOtherTrait {
  use SomeTrait;
}

trait SomeOtherOtherTrait implements SomeSealedInteface {}

