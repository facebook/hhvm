<?hh


<<__EntryPoint>>
function main_systemlib_is_silent() :mixed{
new ArrayIterator(dict[]);
var_dump(idx(dict[], "foo", "bah"));
}
