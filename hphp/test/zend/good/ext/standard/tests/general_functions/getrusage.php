<?hh
<<__EntryPoint>> function main(): void {
var_dump(gettype(getrusage()));
var_dump(gettype(getrusage(1)));
var_dump(gettype(getrusage(-1)));
try { var_dump(getrusage(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


echo "Done\n";
}
