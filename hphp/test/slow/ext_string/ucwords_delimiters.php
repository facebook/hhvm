<?hh


<<__EntryPoint>>
function main_ucwords_delimiters() {
var_dump(ucwords("never|gonna/give.you;up", "|/;"));
var_dump(ucwords('testing ranges', 'a..f'));
}
