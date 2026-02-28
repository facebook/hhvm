<?hh

class G_Cycle {

  public static function a(): void {
    G_Root::cycleRoot();
    self::c();
  }

  public static function b(): void {
    self::a();
  }

  public static function c(): void {
    self::b();
  }
}
