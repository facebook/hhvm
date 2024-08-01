<?hh

class A {
  public static function castThis(mixed $x): ?this {
    return $x is this ? $x : null;
  }
}

enum E: int {
  A = 1;
}

function cast_a(mixed $x): ?A {
  return $x is A ? $x : null;
}

function cast_shape(mixed $x): ?shape('x' => int, 'y' => int) {
  return $x is shape('x' => int, 'y' => int) ? $x : null;
}

function several_cases(mixed $x): void {
  if ($x is A) {
  } else if ($x is ?int) {
  } else if ($x is bool) {
  } else if ($x is E) {
  } else if ($x is (int, int)) {
  }
}
