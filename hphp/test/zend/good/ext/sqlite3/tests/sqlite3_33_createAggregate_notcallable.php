<?php

function aggregate_step ($var) { return $var; }
function aggregate_final ($var) { return $var; }

$db = new SQLite3(':memory:');

$db->createAggregate ('TESTAGGREGATE', 'aggregate_test_step', 'aggregate_final');
$db->createAggregate ('TESTAGGREGATE2', 'aggregate_step', 'aggregate_test_final');
var_dump($db->createAggregate ('TESTAGGREGATE3', 'aggregate_step', 'aggregate_final'));

$db->close();

echo "Done"
?>