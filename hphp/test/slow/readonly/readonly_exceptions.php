<?hh

function test(readonly Exception $e): noreturn {
  throw $e;
}
