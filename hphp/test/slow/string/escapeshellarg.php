<?hh

<<__EntryPoint>>
function main_escapeshellarg() :mixed{
echo strlen(escapeshellarg(str_repeat("AAAAAAAAAAAAAAAA", 89478486)));
}
