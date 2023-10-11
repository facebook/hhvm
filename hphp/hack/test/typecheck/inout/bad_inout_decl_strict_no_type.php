<?hh

function test(inout $x): noreturn {
  throw new Exception('foo');
}
