<?hh

<<__Sealed(SomeRandomClass::class, SomeOtherClass::class)>>
interface SomeSealedInterface {}

class SomeOtherClass implements SomeSealedInterface {}

class SomeOtherOtherClass implements SomeSealedInterface {}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
