<?hh
<<__EntryPoint>> function test(): void {
$db = new SQLite3(':memory:');

echo "SELECTING from invalid table\n";
$result = $db->query("SELECT * FROM non_existent_table");
if (!$result) {
	echo "Error Code: " . $db->lasterrorcode() . "\n";
	echo "Error Msg: " . $db->lasterrormsg() . "\n";
}
echo "Closing database\n";
var_dump($db->close());
echo "Done\n";
}
