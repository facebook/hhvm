<?hh


// No warning
<<__EntryPoint>>
function main_bad_strrpos() {
var_dump(strrpos('', '/', -1));
}
