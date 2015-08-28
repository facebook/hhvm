<?hh

function test() {
  return ImmMap { 0 => 'foo', 1 => 'bar', 1 => 'baz' };
}

var_dump(test());
