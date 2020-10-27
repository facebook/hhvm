<?hh // strict
class A {
  public function FOO(): void {}
}

class B extends A {
  // static function and instance function do not coincide
  public static function foo(): void {}
}
