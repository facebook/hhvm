<?hh
<<__EntryPoint>> function main(): void {
var_dump(libxml_use_internal_errors(false));
var_dump(libxml_use_internal_errors(true));
var_dump(libxml_use_internal_errors());
try { var_dump(libxml_use_internal_errors(new stdClass)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(libxml_get_errors());
var_dump(libxml_get_last_error());

var_dump(libxml_clear_errors());

echo "Done\n";
}
