<?hh

class C {
  public static function f(): void {
    $foo = null;
    nameof await $foo;
  }
}
