<?php

$file = dirname(__FILE__)."/flock.dat";

var_dump(flock());
var_dump(flock("", "", $var));

$fp = fopen($file, "w");
fclose($fp);

var_dump(flock($fp, LOCK_SH|LOCK_NB));

$fp = fopen($file, "w");

var_dump(flock($fp, LOCK_SH|LOCK_NB));
var_dump(flock($fp, LOCK_UN));
var_dump(flock($fp, LOCK_EX));
var_dump(flock($fp, LOCK_UN));

$would = array(1,2,3);
var_dump(flock($fp, LOCK_SH|LOCK_NB, $would));
var_dump($would);
var_dump(flock($fp, LOCK_UN, $would));
var_dump($would);
var_dump(flock($fp, LOCK_EX, $would));
var_dump($would);
var_dump(flock($fp, LOCK_UN, $would));
var_dump($would);

var_dump(flock($fp, -1));
var_dump(flock($fp, 0));

@unlink($file);
echo "Done\n";
?>