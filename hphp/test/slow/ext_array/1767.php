<?hh


<<__EntryPoint>>
function main_1767() :mixed{
var_dump(array_fill(-2, -2, 'pear'));
var_dump(array_combine(varray[1, 2], varray[3]));
var_dump(array_combine(varray[], varray[]));
try { var_dump(array_chunk(1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(array_chunk(varray[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$a = varray[1, 2];
var_dump(asort(inout $a, 100000));
}
