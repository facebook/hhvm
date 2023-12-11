<?hh
/* uses RenameFunction to ensure that all arguments are retained */
class A {
  static function foo() :mixed{
    var_dump(debug_backtrace());
  }

  function bar($a, $b, $c = null) :mixed{
    self::foo();
  }
}

function bar() :mixed{
  $a = new A();
  $a->bar(1, "str", vec[1, 2, 3]);
  hphp_invoke_method($a, "A", "bar", vec[1, 2]);
  hphp_invoke_method($a, "A", "bar", Map {'a' => 1, 'b' => 2});
}
function foo() :mixed{
  call_user_func("bar");
}

function error_handler($errno, $errstr, $errfile, $errline, $errcontext) :mixed{
  // Make sure this function shows up in a backtrace
  var_dump(debug_backtrace());
}

function main() :mixed{
  foo();

  set_error_handler(error_handler<>);
}
<<__EntryPoint>>
function entrypoint_debug_backtrace(): void {
  main();
}
