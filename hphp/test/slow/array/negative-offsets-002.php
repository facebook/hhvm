<?php

<<__EntryPoint>>
function main_negative_offsets_002() {
$a = array();
$a[-2] = 1;
$a[] = 2;
var_dump($a);
}
