<?php

$a = 'ok';
 extract(array('b' => &$a), EXTR_REFS);
 $b = 'no';
 var_dump($a);
