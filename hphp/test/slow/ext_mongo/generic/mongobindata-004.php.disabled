<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'mongobindata');
$coll->drop();

$coll->insert(array('bin' => new MongoBinData('abc', MongoBinData::FUNC)));
$coll->insert(array('bin' => new MongoBinData('def', MongoBinData::BYTE_ARRAY)));
$coll->insert(array('bin' => new MongoBinData('ghi', MongoBinData::UUID)));
$coll->insert(array('bin' => new MongoBinData('jkl', MongoBinData::MD5)));
$coll->insert(array('bin' => new MongoBinData('mno', MongoBinData::CUSTOM)));

$cursor = $coll->find();

foreach ($cursor as $result) {
    printf("Type %d with data \"%s\"\n", $result['bin']->type, $result['bin']->bin);
}
?>