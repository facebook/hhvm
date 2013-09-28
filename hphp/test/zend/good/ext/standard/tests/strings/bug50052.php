<?php
$salt = '$1$f+uslYF01$';
$password = 'test';
echo $salt . "\n";
echo crypt($password,$salt) . "\n";
?>