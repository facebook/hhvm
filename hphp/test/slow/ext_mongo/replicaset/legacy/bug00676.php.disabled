<?php
require_once __DIR__."/../../utils/server.inc";
$m = new_mongo();

$oSemafor = $m->selectDb(dbname())->semafor;
$oSemafor->drop();
$oSemafor->w = 42;
$oSemafor->wtimeout = 30;

try{
        $time = microtime(true);
        $x = $oSemafor->insert(array('createts' => microtime(true)), array('safe' => true));
} catch(MongoCursorException $e){
    var_dump($e->getMessage(), $e->getCode());
}
?>