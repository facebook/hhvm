<?hh

function test(inout string $s): Iterator<int> {
  yield 42;
}
