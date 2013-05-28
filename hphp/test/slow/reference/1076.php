<?php

function f(&$a) {
 $a = 'ok';
}
 $a = array();
 $c = &$a['b'];
 $c = 'ok';
 var_dump($a);
