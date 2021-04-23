<?hh

<<__EntryPoint>>
function main_entry(): void {

  $db = new SQLite3(':memory:');

  $stmt = $db->prepare("SELECT 1");

  var_dump($stmt->close());

  var_dump($db->close());

  print "done";
}
