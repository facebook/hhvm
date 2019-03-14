<?php

try { var_dump(gmp_sub()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_sub("")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_sub("", ""));
try { var_dump(gmp_sub("", "", "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_sub(array(), array()));

var_dump($g = gmp_sub(10000, 10001));
var_dump(gmp_strval($g));
var_dump($g = gmp_sub(10000, -1));
var_dump(gmp_strval($g));
var_dump($g = gmp_sub(10000, new stdclass));
var_dump(gmp_strval($g));
var_dump($g = gmp_sub(new stdclass, 100));
var_dump(gmp_strval($g));

echo "Done\n";
?>
