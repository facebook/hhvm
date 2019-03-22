<?php
$array=array('a','b');

$iterator = new ArrayIterator($array);

try { iterator_to_array(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


try { iterator_to_array($iterator,'test','test'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

iterator_to_array('test','test');

