<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection('phpunit', 'mongobindata');
$coll->drop();

$types = array(
    MongoBinData::FUNC,
    MongoBinData::BYTE_ARRAY,
    MongoBinData::UUID,
    MongoBinData::MD5,
    MongoBinData::CUSTOM
);
foreach($types as $type) {
    $doc = array("bin" => new MongoBinData("asdf", $type));
    $coll->insert($doc);
    var_dump($doc["bin"]->type == $type);
}

$cursor = $coll->find();

foreach ($cursor as $result) {
    var_dump($result["bin"]->type);
}
$coll->drop();
?>