<?php
class m extends Mongo { function __construct() {} }
try {
    $m = new m;
    $m->connect();
} catch(Exception $e) {
    var_dump($e->getMessage());
}
?>
===DONE===
