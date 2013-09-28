<?php
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f,'r');	
var_dump(flock($h, LOCK_SH));
gzclose($h);
?>
===DONE===