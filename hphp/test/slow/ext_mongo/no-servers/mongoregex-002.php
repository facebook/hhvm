<?php
$regex = new MongoRegex('//nope');
var_dump($regex->regex);
var_dump($regex->flags);

$regex = new MongoRegex('//iii');
var_dump($regex->regex);
var_dump($regex->flags);

$regex = new MongoRegex('//nopenope');
var_dump($regex->regex);
var_dump($regex->flags);
?>
