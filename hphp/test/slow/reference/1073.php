<?php


<<__EntryPoint>>
function main_1073() {
$a = array(1, 2);
 foreach ($a as &$b) {
 $b++;
}
 var_dump($a);
}
