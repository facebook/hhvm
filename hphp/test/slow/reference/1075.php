<?php

function f(&$a) {
 $a = 'ok';
}
 $a = 10;
 f($a);
 var_dump($a);
