<?php
parse_str("dummy=x", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);


var_dump(phpcredits());
var_dump(phpcredits(array()));

echo "--\n";
var_dump(phpcredits(0));

echo "--\n";
var_dump(phpcredits(CREDITS_GROUP));

?>