<?hh

<<__EntryPoint>>
function main_high_ascii_label() {
var_dump(eval("return <<<\xff\nXYZ\n\xff\n;"));
var_dump(eval("return <<<'\xff'\nXYZ\n\xff\n;"));
}
