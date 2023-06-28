<?hh

function test() :mixed{
  return ImmMap { 0 => 'foo', 1 => 'bar', 1 => 'baz' };
}


<<__EntryPoint>>
function main_bad_packed_init() :mixed{
var_dump(test());
}
