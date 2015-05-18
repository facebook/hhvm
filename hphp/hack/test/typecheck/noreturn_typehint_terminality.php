<?hh // strict

function my_invariant_violation(string $arg): noreturn {
  throw new Exception($arg);
}

class C {
  public static function invariant_violation(string $arg): noreturn {
    throw new Exception($arg);
  }
}

function foo(?int $x): int {
  if ($x === null) {
    my_invariant_violation("yup!");
  }
  return $x;
}

function bar(?string $x): string {
  if ($x === null) {
    C::invariant_violation("yup!");
  }
  return $x;
}
