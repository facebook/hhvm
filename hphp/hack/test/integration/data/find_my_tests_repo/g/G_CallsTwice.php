<?hh

class G_CallsTwice {
  public static function foo(): void {
    G_Root::duplicateRoot();
  }

  public static function callsFooTwice(): void {
    self::foo();
    self::foo();
  }
}
