<?php
// note that gzclose is an alias to fclose. parameter checking tests will be
// the same as fclose

$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
gzread($h, 20);
var_dump(gzclose($h));

//should fail.
gzread($h, 20);

$h = gzopen($f, 'r');
gzread($h, 20);
var_dump(fclose($h));

//should fail.
gzread($h, 20);


?>
===DONE===