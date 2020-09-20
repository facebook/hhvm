<?hh

<<__Sealed(SomeTrait::class)>>
interface SomeInterface {}

trait SomeTrait2 implements SomeInterface {}

trait SomeTrait {
  use SomeTrait2;
}

