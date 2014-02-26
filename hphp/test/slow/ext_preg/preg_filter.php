<?php
$pattern = array('/\d/', '/[a-z]/', '/[1a]/');
$replace = array('A:$0', 'B:$0', 'C:$0');
$subject = array('1', 'a', '2', 'b', '3', 'A', 'B', '4');
$limit = -1;
$count = -1;

var_dump(preg_filter($pattern, $replace, $subject));
var_dump(preg_filter($pattern, $replace, $subject, $limit));
var_dump(preg_filter($pattern, $replace, $subject, $limit, $count));
var_dump($count);

$subject = '1024';
$count = -1;

var_dump(preg_filter($pattern, $replace, $subject, $limit, $count));
var_dump($count);

$subject = 'XYZ';
$count = -1;

var_dump(preg_filter($pattern, $replace, $subject, $limit, $count));
var_dump($count);

$subject = array('0', '00', '000', '0000', '00000');
$limit = 3;
$count = -1;

var_dump(preg_filter($pattern, $replace, $subject, $limit, $count));
var_dump($count);
