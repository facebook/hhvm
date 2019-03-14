<?php



<<__EntryPoint>>
function main_sqrt() {
try { var_dump(sqrt('foo')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(sqrt('16'));
try { var_dump(sqrt(array())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
