<?hh

class A {
  protected static darray<string,mixed> $a = darray[];

  public function f(bool $b): void {
    while ($b) {
      if (!isset(self::$a["hello"])) {
        return;
      }
    }
  }
}
