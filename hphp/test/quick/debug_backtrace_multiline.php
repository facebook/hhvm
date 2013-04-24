<?hh

function compare_lines($line_from_macro, $line_from_trace) {
  if ($line_from_macro != $line_from_trace) {
    echo "Expected line: ".$line_from_macro."\n";
    echo "Got line: ".$line_from_trace."\n";
  }
}

class A {
  function x($line, $trace_elem = 0) {
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

function newInstanceOfA($line) {
  return (new A())->x($line, 1);
}

// UnboxR
(new A())->x(__LINE__)->
  x(__LINE__)->
  x(__LINE__);

// PopR
(new A())->x(__LINE__)
  ;

// BoxR
$var = &newInstanceOfA(__LINE__);
$var =
  &newInstanceOfA(__LINE__)
  ;
$var =
  &newInstanceOfA(__LINE__);

// handled exception
try {
  throw new Exception(__LINE__);
} catch (Exception $e) {
  compare_lines($e->getMessage(), $e->getLine());
}

// unhandled exception
throw new Exception(__LINE__);
