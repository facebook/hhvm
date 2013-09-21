<?php

$str = "PATHOLOGIES MÉDICO-CHIRUR. ADUL. PL";
$str_iconv = iconv('CP850', 'ISO-8859-1', $str );
var_dump($str_iconv);

?>