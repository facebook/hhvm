<?hh

<<__Sealed(SomeOtherClass::class)>>
class SomeSealedClass {}

class SomeOtherClass extends SomeSealedClass {}

class SomeOtherOtherClass extends SomeSealedClass {}

