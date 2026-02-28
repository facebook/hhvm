<?hh

class HoverWithErrorsClass {
  /** Constructor with doc block */
  public function __construct() {}

  /**
   * During testing, we'll remove the "public" tag from this method
   * to ensure that we can still get IDE services
   */
  public static function staticMethod(string $z): void {}

  /** Static method with doc block */
  public function instanceMethod(int $x1, int $x2): void {
     HoverWithErrorsClass::staticMethod("Hello");
  }
}
