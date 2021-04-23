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
  $result = $db->query("SELECT * FROM test ORDER BY id ASC");
  while ($row = $result->fetcharray(SQLITE3_NUM)) {
  	$totalColumns = $result->numcolumns();
  	for ($i = 0; $i < $totalColumns; $i++) {
  		echo "Name: " . $result->columnname($i) . " - Type: " . $result->columntype($i) . "\n";
  	}
  }
  $result->finalize();

  echo "Closing database\n";
  var_dump($db->close());
  echo "Done\n";
}
