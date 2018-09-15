<?php


<<__EntryPoint>>
function main_1150() {
$a = null;
 extract(array('a' => 'ok'), EXTR_IF_EXISTS);
 var_dump($a);
}
