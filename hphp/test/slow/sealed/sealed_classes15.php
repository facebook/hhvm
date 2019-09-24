<?hh

<<__Sealed(SomeOtherInterface::class)>>
interface SomeSealedInterface {}

interface SomeOtherInterface extends SomeSealedInterface {}

interface SomeOtherOtherInterface extends SomeOtherInterface {}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
