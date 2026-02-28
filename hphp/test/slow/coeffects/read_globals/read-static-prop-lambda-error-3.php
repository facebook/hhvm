<?hh

class Foo {

  <<__LateInit>> public static (function()[]: bool) $fn;

  public function init(): void {
    self::$fn = function()[] { return true;};
  }
}

function call_static_prop_lambda_via_var_with_read_globals(Foo $foo)[read_globals] : void {
  (readonly $foo::$fn)(); // No error
}

function call_static_prop_lambda_via_var(Foo $foo)[] : void {
  ($foo::$fn)(); // Error
}
