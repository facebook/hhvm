<?hh

class Foo {

  <<__LateInit>> public static (function()[]: bool) $fn;

  public function init(): void {
    self::$fn = function()[] { return true;};
  }

  public function call_static_prop_lambda_via_self()[] : void {
    (self::$fn)(); // Error
  }

  public function call_static_prop_lambda_via_self_with_read_globals()[read_globals] : void {
    (self::$fn)(); // No error
  }
}

function call_static_prop_lambda()[] : void {
  (Foo::$fn)(); // Error
}

function call_static_prop_lambda_with_read_globals()[read_globals] : void {
  (Foo::$fn)(); // No error
}

function call_static_prop_lambda_via_var(Foo $foo)[] : void {
  ($foo::$fn)(); // Error
}

function call_static_prop_lambda_via_var_with_read_globals(Foo $foo)[read_globals] : void {
  ($foo::$fn)(); // No error
}
