<?php

<<__EntryPoint>>
function main_htmlent() {
$s = wddx_serialize_value("Test for &");
var_dump($s);
$d = wddx_deserialize($s);
var_dump($d);
}
