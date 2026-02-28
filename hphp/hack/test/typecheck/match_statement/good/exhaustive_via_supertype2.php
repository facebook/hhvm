<?hh

<<file: __EnableUnstableFeatures('case_types')>>
<<file: __EnableUnstableFeatures('match_statements')>>

final class A {}

<<__Sealed(B1::class, B2::class, B3::class)>>
interface IB {}
final class B1 implements IB {}
final class B2 implements IB {}
final class B3 implements IB {}

case type AB1B2 = A | B1 | B2;

function test(AB1B2 $ab): void {
  match ($ab) {
    _: A => hh_expect<A>($ab);
    _: IB => hh_expect<IB>($ab);
  }
}
