<?hh

<<file:__EnableUnstableFeatures('case_types')>>
<<file:__EnableUnstableFeatures('match_statements')>>

final class A {}
final class B {}
case type AB = A | B;

function test(AB $ab): void {
  match ($ab) {
    _: A => print('A');
  }
}
