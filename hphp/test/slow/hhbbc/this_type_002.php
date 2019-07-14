<?hh

abstract class A {
  public static function g(): this {
    // No VerifyRetType because the whole func family returns this.
    return static::f();
  }
  abstract protected static function f(): this;
}

class D extends A {
  protected static function f(): this {
    // This doesn't need a VerifyRetType because it trivially returns this.
    return new static();
  }
  public function h(): this {
    // This needs a VerifyRetNotNull because it will emit a BareThis.
    return $this;
  }
  public function j(): ?this {
    // This doesn't need a VerifyRetType or VerifyRetNotNull.
    return $this;
  }
  public function i(): this {
    // This doesn't need a VerifyRetType or VerifyRetNotNull.
    $a = $this->j();
    if ($a !== null) {
      return $a;
    }
    return $this->h();
  }
}

final class C extends A {
  protected static function f(): this {
    // No VerifyRetType because C is final, so this must construct a C object.
    return new static();
  }
}

final class B extends D {
  public static function f(): this {
    // Must VerifyRetType (this is a clear failure).
    return new C();
  }
  public function h(): this {
    // This needs a VerifyRetNotNull because of the BareThis.
    return $this;
  }
}
<<__EntryPoint>> function main(): void {
$b = new D();
$b->h();
}
