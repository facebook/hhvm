<?hh // partial

class A {
  protected static $a = darray[];

  public function f(bool $b): void {
    while ($b) {
      if (!isset(self::$a["hello"])) {
        return;
      }
    }
  }
}
