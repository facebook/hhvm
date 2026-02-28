<?hh

class A {
  protected static darray<string,mixed> $a = dict[];

  public function f(bool $b): void {
    while ($b) {
      if (!isset(self::$a["hello"])) {
        return;
      }
    }
  }
}
