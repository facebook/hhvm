<?php
require_once __DIR__."/../../utils/server.inc";

$m = new_mongo();

$start = time();

try {
    $m->selectDb(dbname())->test->insert(array("random" => "data"), array("wTimeoutMS" => 1, "w" => 7));
} catch(MongoCursorException $e) {
    var_dump($e->getMessage(), $e->getCode());
}

if ((time() - $start) > 2) {
	echo "timeout longer than it should have been\n";
}
?>