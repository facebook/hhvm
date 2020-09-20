<?hh

<<__Sealed(SomeOtherSealedClass::class)>>
class SomeSealedClass {}

<<__Sealed(SomeNotSealedClass::class)>>
class SomeOtherSealedClass extends SomeSealedClass {}

class SomeNotSealedClass extends SomeOtherSealedClass {}

class SomeIrreleventClass extends SomeNotSealedClass {}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
