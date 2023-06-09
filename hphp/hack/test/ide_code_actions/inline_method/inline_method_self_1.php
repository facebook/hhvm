<?hh

class A {
  private static function inline_me(int $i): string {
    $z = 2;
    return $i + $z;
  }

  public static function foo(): void {
    1 + self::/*range-start*/inline_me/*range-end*/(1);
  }
}
