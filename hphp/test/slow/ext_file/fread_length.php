<?php


<<__EntryPoint>>
function main_fread_length() {
$h = fopen(__FILE__, 'r');
var_dump(fread($h, 0));
}
