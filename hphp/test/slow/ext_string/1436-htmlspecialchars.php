<?php

// Github Issue #1436: Return type of htmlspecialchars() when
// wrong type passed.

$string = array('str'=>'aaa');
$str = htmlspecialchars($string, ENT_QUOTES, 'UTF-8');
var_dump($str); 
