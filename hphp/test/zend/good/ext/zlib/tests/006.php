<?php

var_dump(gzdeflate());
var_dump(gzdeflate("", 1000));
var_dump(gzdeflate("", -1));

var_dump(gzdeflate(""));
var_dump(gzdeflate("", 9));

$string = "Answer me, it can't be so hard
Cry to relieve what's in your heart
Desolation, grief and agony";

var_dump($data1 = gzdeflate($string));
var_dump($data2 = gzdeflate($string, 9));

var_dump(gzinflate());
var_dump(gzinflate(""));
var_dump(gzinflate("asfwe", 1000));
var_dump(gzinflate("asdf", -1));

var_dump(gzinflate("asdf"));
var_dump(gzinflate("asdf", 9));

var_dump(gzinflate($data1));
var_dump(gzinflate($data2));
$data2{4} = 0;
var_dump(gzinflate($data2));

echo "Done\n";
?>