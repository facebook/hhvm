<?php

require_once(dirname(__FILE__) . '/new_db.inc');

var_dump($db);
var_dump($db->close());
echo "Done\n";
?>