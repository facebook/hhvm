<?php

$a = null;
 extract(array('a' => 'ok'), EXTR_IF_EXISTS);
 var_dump($a);
