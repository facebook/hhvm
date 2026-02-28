<?hh

function aggregate_step ($var) :mixed{ return $var; }
function aggregate_final ($var) :mixed{ return $var; }
<<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');

$db->createaggregate ('TESTAGGREGATE', 'aggregate_test_step', 'aggregate_final');
$db->createaggregate ('TESTAGGREGATE2', 'aggregate_step', 'aggregate_test_final');
var_dump($db->createaggregate ('TESTAGGREGATE3', 'aggregate_step', 'aggregate_final'));

$db->close();

echo "Done";
}
