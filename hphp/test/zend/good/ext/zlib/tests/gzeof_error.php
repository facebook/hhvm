<?php

$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
$extra_arg = 'nothing';
try { var_dump(gzeof( $h, $extra_arg ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gzeof() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
gzclose($h)

?>
===DONE===
