<?hh

/* Due to class context forwarding behavior of lsb keyword self
 * it is inconsistent to have self::foo<>() and (self::foo<>)()
 * behaving differently. Because this behavior is inconsistent
 * when used in non-final classes, ban it.
 */

class BaseClass {
  public static function foo(): void {}
}

class Foo extends BaseClass {
  public static function bar(): void {
    self::foo<>;
  }
}
