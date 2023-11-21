<?hh

<<file:__EnableUnstableFeatures('case_types')>>
<<file:__EnableUnstableFeatures('match_statements')>>

final class A {}
final class B {}
case type AB = A | B;

function test(AB $ab): int {
  $x = null;
  do {
    match ($ab) {
      _: A => break;
      _ => {}
    }
    $x = 0;
  } while (coinflip());
  return $x;
}

function coinflip(): bool { return false; }
