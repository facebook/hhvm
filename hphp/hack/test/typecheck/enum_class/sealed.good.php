<?hh

<<__Sealed(ChildBase::class)>>
enum class Base : mixed {}

// No Hack error as expected
enum class ChildBase : mixed extends Base {}
