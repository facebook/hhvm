<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type MultiClass = KeyedContainer<int, int> | I | E | Base;

<<__Sealed(SealedI::class, C2::class)>>
interface Base {}

<<__Sealed(C1::class)>>
interface SealedI extends Base {}

final class C1 implements SealedI {}

final class C2 implements Base {}

interface I {
  public function i(): int;
}

final class E {
  public int $val = 1;
}
