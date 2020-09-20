<?hh
trait XABCTrait {
  public static function fromTrait(): ?int{
    return self::fromTraitInner1();
  }

  public static function fromTraitInner1(): ?int{
    return self::fromTraitInner2();
  }

  public static function fromTraitInner2(): ?int{
    return self::fromTraitInner3();
  }

  public static function fromTraitInner3(): ?int{
    throw new Exception("hello");
  }
}
