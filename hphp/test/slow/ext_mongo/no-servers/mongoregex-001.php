<?php
$regex = new MongoRegex('//');
var_dump($regex->regex);
var_dump($regex->flags);

$regex = new MongoRegex('/\w+/im');
var_dump($regex->regex);
var_dump($regex->flags);

$regex = new MongoRegex('/foo[bar]{3}/i');
var_dump($regex->regex);
var_dump($regex->flags);
?>
