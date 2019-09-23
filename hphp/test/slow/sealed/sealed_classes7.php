<?hh

<<__Sealed(SomeOtherTrait::class, SomeClass::class)>>
trait SomeTrait {}

trait SomeOtherTrait {
  use SomeTrait;
}

trait SomeOtherOtherTrait {
  use SomeOtherTrait;
}

class SomeClass {
  use SomeTrait;
}

class SomeOtherClass extends SomeClass {}

trait someRandomTrait {
  use SomeTrait;
}

class SomeOtherOtherClass {
  use someRandomTrait;
}

class SomeOtherOtherOtherClass {
  use SomeTrait;
}

