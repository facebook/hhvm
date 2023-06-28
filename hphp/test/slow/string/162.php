<?hh


<<__EntryPoint>>
function main_162() :mixed{
var_dump(bin2hex(serialize("a\x00b")));
}
