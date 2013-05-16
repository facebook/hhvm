<?php

$filename = dirname(__FILE__)."/fscanf.dat";

var_dump(fscanf());
var_dump(fscanf(array()));
var_dump(fscanf(array(), array(), new stdclass));

file_put_contents($filename, "data");

$fp = fopen($filename, "rt");
var_dump(fscanf($fp, "%d", $v));
var_dump($v);
fclose($fp);

$fp = fopen($filename, "rt");
var_dump(fscanf($fp, "%s", $v));
var_dump($v);
fclose($fp);

$fp = fopen($filename, "rt");
var_dump(fscanf($fp, "%s", $v, $v1));
var_dump($v);
var_dump($v1);
fclose($fp);

$v = array();
$v1 = array();
$fp = fopen($filename, "rt");
var_dump(fscanf($fp, "", $v, $v1));
var_dump($v);
var_dump($v1);
fclose($fp);

$v = array();
$v1 = array();
$fp = fopen($filename, "rt");
var_dump(fscanf($fp, "%.a", $v, $v1));
var_dump($v);
var_dump($v1);
fclose($fp);

@unlink($filename);
touch($filename);

$fp = fopen($filename, "rt");
var_dump(fscanf($fp, "%s", $v));
var_dump($v);
fclose($fp);

file_put_contents($filename, "data");

$fp = fopen($filename, "rt");
var_dump(fscanf($fp, "%s%d", $v));

@unlink($filename);
echo "Done\n";
?>