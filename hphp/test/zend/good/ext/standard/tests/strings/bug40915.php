<?php

$str = (binary)"a\000z";

var_dump(addslashes($str));
var_dump(addcslashes($str, (binary)""));
var_dump(addcslashes($str, (binary)"\000z"));
var_dump(addcslashes( $str, (binary)"z"));

echo "Done\n";
?>