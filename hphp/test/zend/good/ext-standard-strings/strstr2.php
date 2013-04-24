<?php

$email  = 'aexample.com';
var_dump(strstr($email, '@'));
var_dump(strstr($email, '@', 1));
$email  = 'a@example.com';
var_dump(strstr($email, '@'));
var_dump(strstr($email, '@', 1));
$email  = 'asdfasdfas@e';
var_dump(strstr($email, '@'));
var_dump(strstr($email, '@', 1));
$email  = '@';
var_dump(strstr($email, '@'));
var_dump(strstr($email, '@', 1));
$email  = 'eE@fF';
var_dump(strstr($email, 'e'));
var_dump(strstr($email, 'e', 1));
var_dump(strstr($email, 'E'));
var_dump(strstr($email, 'E', 1));

var_dump(strstr('', ' ', ''));

?>