<?hh

<<file:__EnableUnstableFeatures('case_types')>>
<<file:__EnableUnstableFeatures('match_statements')>>

final class A {}
final class B {}
case type AB = A | B;

function test(AB $ab): void {
  $x = null;
  $y = null;
  $z = null;
  match ($ab) {
    _: A => $x = $ab;
    _: B => $y = $ab;
    _ => $z = $ab;
  }
  hh_expect_equivalent<AB>($ab);
  hh_expect_equivalent<?A>($x);
  hh_expect_equivalent<?B>($y);
  hh_expect_equivalent<null>($z);
}
