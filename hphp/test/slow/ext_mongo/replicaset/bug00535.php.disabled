<?php
require_once __DIR__.'/../utils/server.inc';

MongoLog::setLevel(MongoLog::ALL);
MongoLog::setModule(MongoLog::ALL);
MongoLog::setCallback(function($module, $level, $msg) {
    if (strpos($msg, "command supports") !== false) {
        echo $msg, "\n";
        return;
    }
    if (strpos($msg, "forcing") !== false) {
        echo $msg, "\n";
        return;
    }
});

$rs = MongoShellServer::getReplicasetInfo();
$mc = new MongoClient($rs['dsn'], array('replicaSet' => $rs['rsname']));

$db = $mc->selectDB(dbname());
$collection = $db->selectCollection('bug535');
$collection->drop();

$collection->insert(array('category' => 'fruit', 'name' => 'apple'));
$collection->insert(array('category' => 'fruit', 'name' => 'peach'));
$collection->insert(array('category' => 'fruit', 'name' => 'banana'));
$collection->insert(array('category' => 'veggie', 'name' => 'corn'));
$collection->insert(array('category' => 'veggie', 'name' => 'broccoli'));

echo "\nDone priming data... now the actual tests\n";

echo "\nTesting count\n";
var_dump($collection->count());

echo "\nTesting group\n";
$result = $collection->group(
    array('category' => 1),
    array('items' => array()),
    new MongoCode('function (obj, prev) { prev.items.push(obj.name); }')
);
var_dump($result['ok']);

echo "\nTesting dbStats\n";
$result = $db->command(array('dbStats' => 1));
var_dump($result['ok']);

echo "\nTesting collStats\n";
$result = $db->command(array('collStats' => 'bug535'));
var_dump($result['ok']);

echo "\nTesting distinct\n";
$result = $collection->distinct('category');
var_dump($result);

echo "\nTesting aggregate\n";
$result = $collection->aggregate(array('$match' => array('category' => 'fruit')));
var_dump($result["ok"]);

echo "\nTesting mapreduce\n";
$map = new MongoCode('function() { emit(this.category, 1); }');
$reduce = new MongoCode('function(k, vals) { var sum = 0; for (var i in vals) { sum += vals[i]; } return sum; }');
$result = $db->command(array(
    'mapreduce' => 'bug535',
    'map' => $map,
    'reduce' => $reduce,
    'out' => array('replace' => 'bug535.mapreduce'),
));
var_dump($result['ok']);

echo "\nTesting *inline* mapreduce\n";
$result = $db->command(array(
    'mapreduce' => 'bug535',
    'map' => $map,
    'reduce' => $reduce,
    'out' => 'inline',
));
var_dump($result["ok"]);

?>