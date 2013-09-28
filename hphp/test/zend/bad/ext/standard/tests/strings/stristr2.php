<?php

$email  = 'AbcCdEfGh';
var_dump(stristr($email, 'c'));
var_dump(stristr($email, 'c', 1));

$email  = 'AbCdeEfGh';
var_dump(stristr($email, 'E'));
var_dump(stristr($email, 'E', 1));

$email  = 'wazAbCdeEfGh';
var_dump(stristr($email, 97));
var_dump(stristr($email, 97, 1));

?>