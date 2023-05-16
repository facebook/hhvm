<?hh


<<__EntryPoint>>
function main_sqlite3_null_byte() {
$file = '/etc/passwd'.chr(0).'asdf';

$db = sys_get_temp_dir().'/'.'example.db';

set_error_handler(function() use ($db) { unlink($db); return false; });

$sql3 = new SQLite3($db);
var_dump($sql3->loadExtension($file));

$sql3 = new SQLite3($file);
}
