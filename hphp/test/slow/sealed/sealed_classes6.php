<?hh

namespace SealedClass6;

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

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
