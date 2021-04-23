<?hh
<<__EntryPoint>>
function main_entry(): void {

  $db = new SQLite3(':memory:');

  var_dump($db);
  var_dump($db->close());
  echo "Done\n";
}
