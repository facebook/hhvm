<?hh

<<__Sealed(SomeRandomClass::class, SomeOtherInterface::class)>>
interface SomeSealedInterface {}

interface SomeOtherInterface extends SomeSealedInterface {}

interface SomeOtherOtherInterface extends SomeSealedInterface {}

