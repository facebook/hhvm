<?php
/* 5.2 */
if (!defined("E_DEPRECATED")) {
    define("E_DEPRECATED", E_STRICT);
}
require_once __DIR__."/../utils/server.inc";
error_reporting(-1);

$numNotices = 0;

function handleNotice($errno, $errstr) {
    global $numNotices;
    ++$numNotices;
}

set_error_handler('handleNotice', E_DEPRECATED);

$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'mongobindata');
$coll->drop();

$coll->insert(array('_id' => 1, 'bin' => new MongoBinData('abcdefg')));

$result = $coll->findOne(array('_id' => 1));

echo get_class($result['bin']) . "\n";
echo $result['bin']->bin . "\n";
echo $result['bin']->type . "\n";
var_dump(1 === $numNotices);
?>