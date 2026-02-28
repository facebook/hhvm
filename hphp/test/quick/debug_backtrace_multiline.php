<?hh

function compare_lines($line_from_macro, $line_from_trace) :mixed{
  if (HH\Lib\Legacy_FIXME\neq($line_from_macro, $line_from_trace)) {
    echo "Expected line: ".$line_from_macro."\n";
    echo "Got line: ".$line_from_trace."\n";
  }
}

class A {
  function x($line, $trace_elem = 0) :mixed{
    compare_lines($line, debug_backtrace()[$trace_elem]['line']);
    return $this;
  }
}

class B {
  function __construct($line) {
    compare_lines($line, debug_backtrace()[0]['line']);
  }
}

class C {
}

function newInstanceOfA($line) :mixed{
  return (new A())->x($line, 1);
}

// Box
function box(inout $_) :mixed{}

<<__EntryPoint>>
function main(): void {
  // non-PopC
  (new A())->x(__LINE__)->
    x(__LINE__)->
    x(__LINE__);

  // PopC
  (new A())->x(__LINE__)
    ;

  $x = newInstanceOfA(__LINE__); box(inout $x);

  $x = newInstanceOfA(__LINE__); box(inout $x);


  $x = newInstanceOfA(__LINE__); box(inout $x);

  // handled exception
  try {
    throw new Exception((string)__LINE__);
  } catch (Exception $e) {
    compare_lines($e->getMessage(), $e->getLine());
  }

  // unhandled exception
  throw new Exception((string)__LINE__);
}
