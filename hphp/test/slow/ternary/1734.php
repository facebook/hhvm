<?hh


<<__EntryPoint>>
function main_1734() :mixed{
$a = 123;
try { echo $a ? @mysql_data_seek(null, null) : false; } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
