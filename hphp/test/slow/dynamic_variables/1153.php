<?php

$a = 'ok';
 $arr = array('b' => &$a);
 extract($arr, EXTR_REFS);
 $b = 'no';
 var_dump($a);
