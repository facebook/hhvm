<?php


<<__EntryPoint>>
function main_popen_bad_args() {
$hurr = 'A';
$durr = null;

$res = popen($hurr, $durr);
var_dump($res);
}
