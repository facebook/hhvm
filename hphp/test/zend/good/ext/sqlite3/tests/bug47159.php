<?php

require_once(dirname(__FILE__) . '/new_db.inc');

$stmt = $db->prepare("SELECT 1");

var_dump($stmt->close());

var_dump($db->close());

print "done";

?>