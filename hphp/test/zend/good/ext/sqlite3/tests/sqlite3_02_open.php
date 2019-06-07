<?hh
<<__EntryPoint>> function main() {
try {
  $db = new SQLite3();
} catch (Exception $e) {
  var_dump($e->getMessage());
}
}
