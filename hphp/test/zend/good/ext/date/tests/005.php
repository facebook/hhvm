<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');

$t = mktime(0,0,0, 6, 27, 2006);

try { var_dump(idate()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(idate(1,1,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(idate("1",1));
var_dump(idate(""));
var_dump(idate("0"));

var_dump(idate("B", $t));
var_dump(idate("[", $t));
var_dump(idate("'"));

echo "Done\n";
}
