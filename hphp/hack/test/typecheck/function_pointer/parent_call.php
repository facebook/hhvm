<?hh

/* Due to class context forwarding behavior of lsb keyword parent
 * it is inconsistent to have parent::foo<>() and (parent::foo<>)()
 * behaving differently. Ban it.
 */

class BaseClass {
  public static function foo(): void {}
}

final class Foo extends BaseClass {
  public static function bar(): void {
    parent::foo<>;
  }
}
