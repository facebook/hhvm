<?php require_once __DIR__."/../utils/server.inc";

$mongo = new_mongo_standalone();

$db = $mongo->selectDB(dbname());

class foo {
    function __toString() {
        return "foo";
    }
}
$f = new foo;
$m = new MongoCollection($db, $f);
var_dump($m);

$m2 = new MongoCollection($db, $m);
var_dump($m2);

$col1 = $db->selectCollection($f);
var_dump($col1);

?>
===DONE===
<?php exit(0);?>