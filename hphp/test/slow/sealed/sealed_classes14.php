<?hh

<<__Sealed(SomeOtherClass::class)>>
class SomeSealedClass {}

class SomeOtherClass extends SomeSealedClass {}

class SomeOtherOtherClass extends SomeOtherClass {}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
