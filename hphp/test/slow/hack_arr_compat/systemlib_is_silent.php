<?hh


<<__EntryPoint>>
function main_systemlib_is_silent() :mixed{
new ArrayIterator(darray[]);
var_dump(idx(darray[], "foo", "bah"));
}
