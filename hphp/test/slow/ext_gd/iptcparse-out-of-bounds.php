<?hh


<<__EntryPoint>>
function main_iptcparse_out_of_bounds() :mixed{
iptcparse("\x1C\x02_\x80___");
}
