<?php
// Test assignments using (positive and negative) string offsets

$str = "abcdefghijklmno";
$i = 3;
$j = -4;

$str{2} = 'C';
var_dump($str);

$str{$i} = 'Z';
var_dump($str);

$str{-5} = 'P';
var_dump($str);

$str{$j} = 'Q';
var_dump($str);

$str{-20} = 'Y';
var_dump($str);

$str{-strlen($str)} = strtoupper($str{0}); /* An exotic ucfirst() ;) */
var_dump($str);

$str{20} = 'N';
var_dump($str);

$str{-2} = 'UFO';
var_dump($str);

$str{-$i} = $str{$j*2};
var_dump($str);
?>
