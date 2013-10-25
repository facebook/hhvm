<?php
require_once __DIR__."/../../utils/server.inc";

$cfg = MongoShellServer::getShardInfo();
$m = new MongoClient($cfg[0]);

$m->setReadPreference(MongoClient::RP_PRIMARY);
$cursor = $m->selectCollection('phpunit', 'c')->find();
$cursor->explain();
$info = $cursor->info();
// No $readPreference for "primary", since that is the default already
var_dump(!isset($info['query']['$readPreference']['mode']));

$m->setReadPreference(MongoClient::RP_PRIMARY_PREFERRED);
$cursor = $m->selectCollection('phpunit', 'c')->find();
$cursor->explain();
$info = $cursor->info();
var_dump(MongoClient::RP_PRIMARY_PREFERRED === $info['query']['$readPreference']['mode']);

$m->setReadPreference(MongoClient::RP_SECONDARY);
$cursor = $m->selectCollection('phpunit', 'c')->find();
$cursor->explain();
$info = $cursor->info();
var_dump(MongoClient::RP_SECONDARY === $info['query']['$readPreference']['mode']);

$m->setReadPreference(MongoClient::RP_SECONDARY_PREFERRED);
$cursor = $m->selectCollection('phpunit', 'c')->find();
$cursor->explain();
$info = $cursor->info();
// No $readPreference for "secondaryPreferred" unless tags were specified
var_dump(!isset($info['query']['$readPreference']['mode']));

$m->setReadPreference(MongoClient::RP_NEAREST);
$cursor = $m->selectCollection('phpunit', 'c')->find();
$cursor->explain();
$info = $cursor->info();
var_dump(MongoClient::RP_NEAREST === $info['query']['$readPreference']['mode']);
?>