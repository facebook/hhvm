<?hh
require_once("hphp/test/quick/traits_exception.inc");

final class XABC {
  use XABCTrait;
  public static function fromClass (): ?int{
    return self::fromTrait();
  }
}

<<__EntryPoint>>
function main() {
  echo XABC::fromClass();
}
