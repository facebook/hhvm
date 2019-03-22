<?php

echo "\n*** Testing error conditions ***";
var_dump(array_keys(100));
var_dump(array_keys("string"));
var_dump(array_keys(new stdclass));  // object
try { var_dump(array_keys()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero arguments
try { var_dump(array_keys(array(), "", TRUE, 100));  } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected
var_dump(array_keys(array(1,2,3, array() => array())));  // (W)illegal offset

echo "Done\n";
