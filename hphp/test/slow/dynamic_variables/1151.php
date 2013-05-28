<?php

$a = null;
 extract(array('a' => 'ok', 'b' => 'no'), EXTR_PREFIX_IF_EXISTS, 'p');
 var_dump($p_a);
 var_dump($b);
 var_dump($p_b);
