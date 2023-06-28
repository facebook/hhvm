<?hh


<<__EntryPoint>>
function main_errmode() :mixed{
$db = sys_get_temp_dir().'/'.'errmode.php.sq3';

$pdo = new PDO("sqlite:$db");
$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING);
$pdo->setAttribute(PDO::ATTR_ERRMODE, 77);
$pdo->exec('this is not a query');
unset($pdo);
unlink($db);
}
