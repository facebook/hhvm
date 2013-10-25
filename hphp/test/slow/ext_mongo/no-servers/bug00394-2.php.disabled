<?php
require_once __DIR__."/../utils/server.inc";

class dummy {}

function errhandler() {
       unset($GLOBALS['arr1'][0]);
       return true;
}

$arr1 = array(new dummy, 1);
$oldhandler = set_error_handler("errhandler");

$x = new MongoCollection;
if ($x) {
    $x->batchInsert(&$arr1, $info);
}
restore_error_handler();

echo "I am alive\n";
?>
===DONE===