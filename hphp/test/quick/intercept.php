<?hh

function foo() { var_dump(__METHOD__); }
function bar($_1, $_2, inout $_3, $_4, inout $_5) {
  var_dump(__METHOD__);
  throw new Exception;
}

<<__EntryPoint>> function boo(): void {
  fb_intercept('foo', 'bar', 'bar');
  try {
    foo();
  } catch (Exception $e) {
    var_dump("caught:" . $e->getMessage());
  }
}
