<?hh

class A {
  const type T = shape(nameof A => int);

  public static function read(shape(nameof A => int) $s): int {
    return $s[nameof self];
  }
}

class B extends A {
  public static function explicit_base(this::T $s): int {
    return $s[nameof A];
  }

  public static function inherited_type(this::T $s): int {
    return $s[nameof self];
  }
}

function inherited(shape(nameof A => int) $s): int {
  return B::read($s);
}
