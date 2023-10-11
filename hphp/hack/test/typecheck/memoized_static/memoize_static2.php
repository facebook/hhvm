<?hh
final class FinalC implements IMemoizeParam {
  <<__Memoize>>
  public static function get(): this {
    return new static();
  }
  public function getInstanceKey():string {
    return "";
  }
}
