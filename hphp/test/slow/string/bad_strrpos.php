<?hh


// No warning
<<__EntryPoint>>
function main_bad_strrpos() :mixed{
var_dump(strrpos('', '/', -1));
}
