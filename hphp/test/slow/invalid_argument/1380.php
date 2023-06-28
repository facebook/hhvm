<?hh


<<__EntryPoint>>
function main_1380() :mixed{
try { var_dump(mysql_fetch_array(null, 0)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(mysql_fetch_object(null, 'stdClass'));
}
