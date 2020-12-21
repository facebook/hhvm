<?hh

<<__Sealed(SomeRandomClass::class, SomeOtherInterface::class)>>
interface SomeSealedInterface {}

interface SomeOtherInterface extends SomeSealedInterface {}

interface SomeOtherOtherInterface extends SomeSealedInterface {}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
