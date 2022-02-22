<?hh

<<__Sealed(ChildBase::class)>>
enum class Base : mixed {}

class ChildBase {}

// Hack error as expected
enum class Foo : mixed extends Base {}
