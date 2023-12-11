<?hh <<__EntryPoint>> function main(): void {
$array=vec['a','b'];

$iterator = new ArrayIterator($array);

try { iterator_to_array(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


try { iterator_to_array($iterator,true,false); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

try { iterator_to_array('test','test'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
