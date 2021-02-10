<?hh

<<__Sealed(ChildBase::class)>>
enum class Base : mixed {}

// Hack error as expected
enum class Foo : mixed extends Base {}
