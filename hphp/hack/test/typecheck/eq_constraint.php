<?hh // strict

class Foo {
  public function assertIsFoo(): void {}
}

class Boxing<Tclass> {
  public function __construct(
    private Tclass $member,
  ) {}

  public function getWithDefault<Tinner>(
    Tinner $default,
  ): Tinner where
    Tclass as ?Tinner,
  {
    if ($this->member !== null) {
      return $this->member;
    } else {
      return $default;
    }
    // This doesn't work because non-null-ness is reflected in the
    // type parameter environment, which isn't merged
    return $this->member !== null ? $this->member : $default;
  }

  public function getOrExcept<Tinner>(): Tinner where Tclass = ?Tinner {
    if ($this->member === null) {
      throw new Exception('Member is null and can\'t be got.');
    }
    return $this->member;
  }

  public function getAsIs(): Tclass {
    return $this->member;
  }

  public static function caller(Boxing<?Foo> $c, Foo $f): void {
    $c->getWithDefault($f)->assertIsFoo(); // Must succeed
    $c->getOrExcept()->assertIsFoo();      // Must succeed
    $c->getAsIs()->assertIsFoo();          // Must fail, because may be null
  }
}
