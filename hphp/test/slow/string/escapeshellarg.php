<?hh

<<__EntryPoint>>
function main_escapeshellarg() {
echo strlen(escapeshellarg(str_repeat("AAAAAAAAAAAAAAAA", 89478486)));
}
