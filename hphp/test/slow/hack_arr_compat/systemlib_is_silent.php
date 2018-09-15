<?hh


<<__EntryPoint>>
function main_systemlib_is_silent() {
new ArrayIterator(array());
var_dump(idx(array(), "foo", "bah"));
}
