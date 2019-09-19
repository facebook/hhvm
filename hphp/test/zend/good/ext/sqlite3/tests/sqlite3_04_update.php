<?hh
<<__EntryPoint>>
function main_entry(): void {

  require_once(dirname(__FILE__) . '/new_db.inc');
  $timenow = time();

  echo "Creating Table\n";
  var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

  echo "INSERT into table\n";
  var_dump($db->exec("INSERT INTO test (time, id) VALUES (" . $timenow . ", 'a')"));
  var_dump($db->exec("INSERT INTO test (time, id) VALUES (" . $timenow . ", 'b')"));

  echo "SELECTING results\n";
  $results = $db->query("SELECT * FROM test ORDER BY id ASC");
  while ($result = $results->fetchArray(SQLITE3_NUM))
  {
  	var_dump($result);
  }
  $results->finalize();

  echo "UPDATING results\n";
  var_dump($db->exec("UPDATE test SET id = 'c' WHERE id = 'a'"));

  echo "Checking results\n";
  $results = $db->query("SELECT * FROM test ORDER BY id ASC");
  while ($result = $results->fetchArray(SQLITE3_NUM))
  {
  	var_dump($result);
  }
  $results->finalize();

  echo "Closing database\n";
  var_dump($db->close());
  echo "Done\n";
}
