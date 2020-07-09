<?hh

final class XABC {
  use XABCTrait;
  public static function fromClass (): ?int{
    return self::fromTrait();
  }
}
