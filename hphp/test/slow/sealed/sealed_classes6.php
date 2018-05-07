<?hh

<<__Sealed(SomeClass::class)>>
interface SomeSealedInteface {}

trait SomeTrait implements SomeSealedInteface {}

class SomeClass {
  use SomeTrait;
}

class SomeOtherClass {
  use SomeTrait;
}
