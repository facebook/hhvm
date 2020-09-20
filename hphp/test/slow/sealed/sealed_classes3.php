<?hh

<<__Sealed(SomeRandomClass::class, SomeOtherClass::class)>>
interface SomeSealedInterface {}

class SomeOtherClass implements SomeSealedInterface {}

class SomeOtherOtherClass implements SomeSealedInterface {}

