<?hh

class Foo {

  <<__LateInit>> public static (function()[]: bool) $fn;

  public function init(): void {
    self::$fn = function()[] { return true;};
  }
}

function call_static_prop_lambda_with_read_globals()[read_globals] : void {
  (Foo::$fn)(); // No error
}

function call_static_prop_lambda()[] : void {
  (Foo::$fn)(); // Error
}
