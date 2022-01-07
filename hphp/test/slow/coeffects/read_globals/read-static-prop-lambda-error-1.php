<?hh

class Foo {

  <<__LateInit>> public static (function()[]: bool) $fn;

  public function init(): void {
    self::$fn = function()[] { return true;};
  }

  public function call_static_prop_lambda_via_self_with_read_globals()[read_globals] : void {
    (self::$fn)(); // No error
  }

  public function call_static_prop_lambda_via_self()[] : void {
    (self::$fn)(); // Error
  }
}
