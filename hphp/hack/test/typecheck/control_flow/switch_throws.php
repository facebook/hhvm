<?hh

function takes_nothing(nothing $x): void {}

function nonexhaustive(int $x): int {
  try {
    $y = 3;
    switch ($x) {
    case 0:
      return 1;
    }
  } catch (Exception $_) {
    // error, $y must have type int
    // because the switch statement
    // may throw
    takes_nothing($y);
  }
  return 0;
}

function exhaustive(int $x): int {
  try {
    $y = 3;
    switch ($x) {
    case 0:
      return 1;
    default:
      return 1;
    }
  } catch (Exception $_) {
    // fine, the switch statement
    // won't throw
    takes_nothing($y);
  }
  return 0;
}
