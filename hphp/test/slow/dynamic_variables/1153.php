<?php


<<__EntryPoint>>
function main_1153() {
$a = 'ok';
 $arr = array('b' => &$a);
 extract($arr, EXTR_REFS);
 $b = 'no';
 var_dump($a);
}
