<?hh

function test() {
  return ImmMap { 0 => 'foo', 1 => 'bar', 1 => 'baz' };
}


<<__EntryPoint>>
function main_bad_packed_init() {
var_dump(test());
}
