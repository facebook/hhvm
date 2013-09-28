<?php

$a = 1;
 extract(array('a' => 'ok'), EXTR_PREFIX_SAME, 'p');
 var_dump($p_a);
