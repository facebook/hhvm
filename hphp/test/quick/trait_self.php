<?hh

trait T {
  static function foo() { self::bar(); }
  static function bar() { var_dump(__METHOD__); }
}
<<__EntryPoint>> function main(): void {
T::foo();
}
