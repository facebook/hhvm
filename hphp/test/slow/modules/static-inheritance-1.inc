<?hh

class Foo {
  public static function get(): int {
    return 0;
  }

  public function get2(): int {
    return 0;
  }

  public function doStuff(): void {
    var_dump(static::get());
  }

  public function doStuff2(): Foo {
    return new static;
  }
}
