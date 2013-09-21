<?php
$crap = 'AåBäCöDü';
var_dump(mb_strcut($crap, 0, 100, 'UTF-8'));
var_dump(mb_strcut($crap, 1, 100, 'UTF-8'));
var_dump(mb_strcut($crap, 2, 100, 'UTF-8'));
var_dump(mb_strcut($crap, 3, 100, 'UTF-8'));
var_dump(mb_strcut($crap, 12, 100, 'UTF-8'));
var_dump(mb_strcut($crap, 13, 100, 'UTF-8'));
?>