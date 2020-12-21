<?hh

<<__Sealed(SomeOtherClass::class)>>
class SomeSealedClass {}

class SomeOtherClass extends SomeSealedClass {}

class SomeOtherOtherClass extends SomeSealedClass {}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
