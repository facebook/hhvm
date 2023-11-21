<?hh

<<file:__EnableUnstableFeatures('case_types')>>
<<file:__EnableUnstableFeatures('match_statements')>>

final class A {}

<<__Sealed(B1::class, B2::class)>>
interface IB {}
final class B1 implements IB {}
final class B2 implements IB {}

case type AB = A | IB;

function test(AB $ab): void {
  match ($ab) {
    _: A => hh_expect<A>($ab);
    _: B1 => hh_expect<B1>($ab);
    _: B2 => hh_expect<B2>($ab);
  }
}
