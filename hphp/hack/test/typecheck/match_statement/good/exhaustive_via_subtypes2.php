<?hh

<<file: __EnableUnstableFeatures('case_types')>>
<<file: __EnableUnstableFeatures('match_statements')>>

final class A {}

<<__Sealed(B1::class, B2::class, IC::class)>>
interface IB {}
final class B1 implements IB {}
final class B2 implements IB {}

<<__Sealed(C1::class, C2::class)>>
interface IC extends IB {}
final class C1 implements IC {}
final class C2 implements IC {}

case type AB = A | IB;

function test(AB $ab): void {
  match ($ab) {
    _: A => hh_expect<A>($ab);
    _: B1 => hh_expect<B1>($ab);
    _: B2 => hh_expect<B2>($ab);
    _: C1 => hh_expect<C1>($ab);
    _: C2 => hh_expect<C2>($ab);
  }
}
