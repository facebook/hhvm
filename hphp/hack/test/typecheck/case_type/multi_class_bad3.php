<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type MultiClass = IC | I;

<<__Sealed(C::class)>>
interface IC {
}

class C implements IC {
}

<<__Sealed(IB::class)>>
interface I {}

<<__Sealed(B::class)>>
interface IB {
}

class B extends C implements IB {}
