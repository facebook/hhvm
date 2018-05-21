<?hh

<<__Sealed(SomeOtherInterface::class)>>
interface SomeSealedInterface {}

interface SomeOtherInterface extends SomeSealedInterface {}

interface SomeOtherOtherInterface extends SomeOtherInterface {}
