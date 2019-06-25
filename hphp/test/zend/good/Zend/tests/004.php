<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(strncmp("", "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(strncmp("", "", 100));
var_dump(strncmp("aef", "dfsgbdf", -1));
var_dump(strncmp("fghjkl", "qwer", 0));
var_dump(strncmp("qwerty", "qwerty123", 6));
var_dump(strncmp("qwerty", "qwerty123", 7));

echo "Done\n";
}
