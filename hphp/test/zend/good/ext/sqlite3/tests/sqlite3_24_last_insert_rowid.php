<?hh
<<__EntryPoint>>
function main_entry(): void {

  require_once(dirname(__FILE__) . '/new_db.inc');
  $timenow = time();

  echo "Creating Table\n";
  var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

  echo "INSERT into table\n";
  var_dump($db->exec("INSERT INTO test (time, id) VALUES (" . $timenow . ", 'a')"));
  var_dump($db->lastInsertRowID());
  var_dump($db->exec("INSERT INTO test (time, id) VALUES (" . $timenow . ", 'b')"));
  var_dump($db->lastInsertRowID());

  echo "Closing database\n";
  var_dump($db->close());
  echo "Done\n";
}
