<?php
$mc = new MongoClient("", array("connect" => false));
var_dump($mc->__toString());
?>
