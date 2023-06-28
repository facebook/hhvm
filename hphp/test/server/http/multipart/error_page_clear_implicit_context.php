<?hh

// FILE: error_page.php

<<__EntryPoint>>
function main_error() :mixed{
  echo "Reading implicit context\n";
  var_dump(IntContext::get());
}

// FILE: main.php

abstract final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static function start<T>(int $context, mixed $f)[zoned, ctx $f]: T {
    return parent::runWith($context, $f);
  }
  public static function get()[zoned]: ?int {
    return parent::get();
  }
}

function foo() :mixed{
  var_dump(IntContext::get());
  hphp_throw_fatal_error("Throwing fatal!");
}

<<__EntryPoint>>
function main(): void {
  echo "Begin!\n";
  hphp_set_error_page("error_page.php");
  register_shutdown_function(function () { IntContext::start(5, foo<>); });
}
