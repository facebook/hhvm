<?hh
<<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');

try {
  $db->loadExtension("");
} catch (Extension $ex) {
  var_dump($ex->getMessage());
}
}
