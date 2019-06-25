<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');

try { var_dump(checkdate()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(checkdate(1,1,1));

var_dump(checkdate(2,29,2006));
var_dump(checkdate(31,6,2006));
var_dump(checkdate(1,1,32768));
var_dump(checkdate(1,1,32767));

var_dump(checkdate(-1,1,2006));
var_dump(checkdate(1,-1,2006));
var_dump(checkdate(1,1,-1));

echo "Done\n";
}
