<?hh

abstract final class AbstractFinalClassWithWrongProperty {
  public int $x;
}

abstract final class AbstractFinalClassWithRightProperty {
  public static int $x;
}

abstract final class AbstractFinalClassWithWrongMethod {
  public function x(): void {}
}

abstract final class AbstractFinalClassWithRightMethod {
  public static function x(): void {}
}
