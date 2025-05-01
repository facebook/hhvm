<?hh

final class Reified<reify T> {}

function test<reify T>(Reified<T> $x, int $y, T $t): int {
  $x_or_y = $y > 0 ? $x : $y;
  if ($x_or_y is Reified<int>) {
    return $t; // we should know T = int
  } else {
    return
      $x_or_y; // should error because $x_or_y could be e.g. Reified<string>
  }
}
