<?php
$tmp = __DIR__ . "/bug65915.inc.php";

file_put_contents($tmp, '<?php return function(){ return "a";};');
$f = require $tmp;
var_dump($f());

opcache_invalidate($tmp, true);

file_put_contents($tmp, '<?php return function(){ return "b";};');
$f = require $tmp;
var_dump($f());

@unlink($tmp);
?>