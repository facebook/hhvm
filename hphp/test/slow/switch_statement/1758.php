<?php

switch ($_POST) {
case array(): echo 'empty array';
 break;
case $_GET:   echo 'get';
 break;
default: echo 'default';
}
switch ($GLOBALS) {
case array(): echo 'empty array';
 break;
default: echo 'default';
}
function ret_true($x) {
 return true;
 }
switch ($GLOBALS) {
case ret_true($GLOBALS['foo'] = 10): echo '1';
 break;
case array();
 echo '2';
 break;
default: echo '3';
}
var_dump($foo);
