<?hh

// Using 'this' as an argument is kind of strange. In order for it to be safe
// you need to invoke a static method on the instance.
<<__ConsistentConstruct>>
class C {
  public static function thisAsArg(this $c): void {}

  public function test(C $c): void {
    $c::thisAsArg($c);

    $new_c = new C();
    C::thisAsArg($new_c);

    $static_c = new static();
    static::thisAsArg($static_c);
    self::thisAsArg($static_c);

    // Error because we cannot prove $c is exactly the type 'C'
    C::thisAsArg($c);
  }
}
