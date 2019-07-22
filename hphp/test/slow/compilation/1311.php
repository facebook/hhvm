<?hh

if (__hhvm_intrinsics\launder_value(0)) {
  include '1311-1.inc';
} else {
  include '1311-2.inc';
}
abstract class x extends y {
  private static $nextSerial = 1;
  private $serial = 0;
  public function __construct() {
    $this->serial = self::$nextSerial++;
  }
}
