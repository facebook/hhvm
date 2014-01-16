<?php

$string = '<body>Umlauttest öüä</body>';

var_dump(strlen($string));
var_dump(mb_strlen($string));

var_dump(strripos($string, '</body>'));
var_dump(mb_strripos($string, '</body>'));

?>