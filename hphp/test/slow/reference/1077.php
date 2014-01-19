<?php

function f(&$a) {
 $a = 'ok';
}
 $a = array();
 $c = &$a['b'];
 f($c);
 var_dump($a);
