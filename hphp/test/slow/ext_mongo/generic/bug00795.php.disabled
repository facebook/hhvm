<?php
function errh($errno, $errmsg) {
    echo $errmsg, "\n";
    return true;
}
set_error_handler("errh", E_RECOVERABLE_ERROR);


$m = new MongoCode("bacd");
$m->code = new stdclass;
var_dump((string)$m);
?>