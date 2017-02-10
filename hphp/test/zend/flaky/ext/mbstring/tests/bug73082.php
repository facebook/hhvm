<?php

ini_set('memory_limit', -1);

$str = str_repeat("a", 0xffffffff/9); //a -> &#97; output_len = input_len*5 -> overflow integer

var_dump(strlen($str));

$str1 = mb_encode_numericentity ($str, array (0x0, 0xffff, 0, 0xffff), 'UTF-8');
?>
