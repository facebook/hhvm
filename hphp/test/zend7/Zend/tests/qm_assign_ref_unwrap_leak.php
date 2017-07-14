<?php

function &ref() {
    $str = "str";
    $str .= "str";
    return $str;
}

var_dump(true ? ref() : ref());
var_dump(ref() ?: ref());
var_dump(ref() ?? ref());

?>
