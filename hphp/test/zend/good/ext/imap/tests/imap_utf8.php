<?hh
<<__EntryPoint>> function main(): void {
var_dump(imap_utf8(""));
var_dump(imap_utf8('1'));
try { var_dump(imap_utf8(vec[1,2])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(imap_utf8("test"));

echo "Done\n";
}
