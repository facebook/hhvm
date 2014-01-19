<?php

function f(&$a) {
}
 $a = array();
 f($a['b']);
 var_dump($a);
