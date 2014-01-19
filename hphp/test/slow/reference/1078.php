<?php

function f(&$a) {
 $a = 'ok';
}
 $a = array();
 f($a['b']);
 var_dump($a);
