<?php
$str1 = 'a';
$str2 = 'b';

$array=Array("1",2,3,1.1,FALSE,NULL,Array(), $str1, &$str2);

var_dump($array);

var_dump(preg_grep('/do not match/',$array));

$a = preg_grep('/./',$array);
var_dump($a);

$str1 = 'x';
$str2 = 'y';

var_dump($a); // check if array is still ok

var_dump($array);

?>