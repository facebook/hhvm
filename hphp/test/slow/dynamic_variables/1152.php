<?php


<<__EntryPoint>>
function main_1152() {
$a = 'ok';
 extract(array('b' => &$a), EXTR_REFS);
 $b = 'no';
 var_dump($a);
}
