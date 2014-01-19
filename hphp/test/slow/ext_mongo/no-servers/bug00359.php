<?php

$id = new MongoId;
if (getmypid() <= 65536) {
    $retval = $id->getPid() == getmypid();
} else {
    $retval = true;
}
var_dump($id->getPid(), $id->__toString(), $retval);

$id = new MongoID("4fe3420a44415ecc83000000");
var_dump($id->getPid(), $id->__toString());

$id = new MongoID("4fe3427744415e4f84000001");
var_dump($id->getPid(), $id->__toString());

$id = new MongoID("4fe342a944415e5284000000");
var_dump($id->getPid(), $id->__toString());
?>
