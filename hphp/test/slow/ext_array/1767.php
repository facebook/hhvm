<?php


<<__EntryPoint>>
function main_1767() {
var_dump(array_fill(-2, -2, 'pear'));
var_dump(array_combine(array(1, 2), array(3)));
var_dump(array_combine(array(), array()));
try { var_dump(array_chunk(1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(array_chunk(array())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$a = array(1, 2);
var_dump(asort(&$a, 100000));
}
