<?hh

<<file:__EnableUnstableFeatures('match_statements')>>
<<file:__EnableUnstableFeatures('case_types')>>

class A {}
class B {}
case type AB = A | B;

function test<Terased, reify Treified, Tbounded as AB>(
  mixed $mixed,
  nonnull $nonnull,
  Terased $erased,
  Treified $reified,
  Tbounded $bounded,
  vec<int> $vec,
  dict<arraykey, int> $dict,
  Container<int> $container,
  (function(int): int) $function,
  (int, int) $tuple,
  shape('x' => int) $shape,
): void {
  match ($mixed) { _ => {} }
  match ($nonnull) { _ => {} }
  match ($erased) { _ => {} }
  match ($reified) { _ => {} }
  match ($bounded) { _ => {} }
  match ($vec) { _ => {} }
  match ($dict) { _ => {} }
  match ($container) { _ => {} }
  match ($function) { _ => {} }
  match ($tuple) { _ => {} }
  match ($shape) { _ => {} }
  match (coinflip() ? 0 : $vec) { _ => {} }
}

function coinflip(): bool { return false; }
