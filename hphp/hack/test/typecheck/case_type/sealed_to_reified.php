<?hh

<<__Sealed(ReifiedFoo::class)>>
interface IFoo {}

abstract class ReifiedFoo<reify T> implements IFoo {}

case type CT = IFoo | ReifiedFoo<int>;
