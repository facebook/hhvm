<?hh


<<__EntryPoint>>
function main_162() {
var_dump(bin2hex(serialize("a\x00b")));
}
