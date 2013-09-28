<?php

$a = 1;
 extract(array('a' => 'ok'), EXTR_SKIP);
 var_dump($a);
