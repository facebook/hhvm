<?hh <<__EntryPoint>> function main(): void {
$array=vec['a','b'];

$iterator = new ArrayIterator($array);

try { iterator_count(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


try { iterator_count($iterator,'1'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

iterator_count('1');
}
