<?hh


<<__EntryPoint>>
function main_nullable_coerce() :mixed{
error_reporting(0);
var_dump(error_reporting(null));
var_dump(error_reporting());
error_reporting(-1);
var_dump(error_reporting(null));
var_dump(error_reporting());
try { var_dump(error_reporting("Banana")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
