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

case type ABC = A | B1 | B2 | C1 | C2;

function test(ABC $ab): void {
  match ($ab) {
    _: A => hh_expect<A>($ab);
    _: IB => hh_expect<IB>($ab);
  }
}
