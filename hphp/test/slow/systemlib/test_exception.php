<?hh

function testException(): void {
  $e = new DivisionByZeroException("Something went wrong", 500);
  printf("{$e::getTraceOptions()}\n");
  $e::setTraceOptions(5);
  printf("{$e::getTraceOptions()}\n");
  printf("{$e->getMessage()}\n");
}

<<__EntryPoint>>
function main(): void {
  testException();
}
