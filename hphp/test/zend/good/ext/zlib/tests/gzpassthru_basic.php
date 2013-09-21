<?php

// note that gzpassthru is an alias to fpassthru. parameter checking tests will be
// the same as fpassthru

$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
var_dump(gzpassthru($h));
var_dump(gzpassthru($h));
gzclose($h);

?>
===DONE===