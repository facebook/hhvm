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

  echo "UPDATE query\n";
  var_dump($db->exec("UPDATE test SET id = 'c'"));

  echo "Rows Updated\n";
  var_dump($db->changes());

  echo "Closing database\n";
  var_dump($db->close());
  echo "Done\n";
}
