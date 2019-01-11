<?php

<<__EntryPoint>>
function main_next() {
$transport = array("foot", "bike", "car", "plane");
var_dump(current(&$transport));
var_dump(next(&$transport));
var_dump(next(&$transport));
var_dump(prev(&$transport));
var_dump(end(&$transport));
}
