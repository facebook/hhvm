<?hh
<<__EntryPoint>>
function main_entry(): void {

  $db = new SQLite3(':memory:');
  $timenow = time();

  echo "Creating Table\n";
  var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

  echo "INSERT into table\n";
  var_dump($db->exec("INSERT INTO test (time, id) VALUES (" . $timenow . ", 'a')"));
  var_dump($db->exec("INSERT INTO test (time, id) VALUES (" . $timenow . ", 'b')"));

  echo "SELECTING results\n";
  var_dump($db->querysingle("SELECT id FROM test WHERE id = 'a'"));
  var_dump($db->querysingle("SELECT id, time FROM test WHERE id = 'a'", true));

  echo "Done";
}
