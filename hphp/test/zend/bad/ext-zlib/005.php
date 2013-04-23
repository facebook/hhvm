<?php

var_dump(gzcompress());
var_dump(gzcompress("", 1000));
var_dump(gzcompress("", -1));

var_dump(gzcompress(""));
var_dump(gzcompress("", 9));

$string = "Answer me, it can't be so hard
Cry to relieve what's in your heart
Desolation, grief and agony";

var_dump($data1 = gzcompress($string));
var_dump($data2 = gzcompress($string, 9));

var_dump(gzuncompress());
var_dump(gzuncompress("", 1000));
var_dump(gzuncompress("", -1));

var_dump(gzuncompress(""));
var_dump(gzuncompress("", 9));

var_dump(gzuncompress($data1));
var_dump(gzuncompress($data2));
$data2{4} = 0;
var_dump(gzuncompress($data2));

echo "Done\n";
?>