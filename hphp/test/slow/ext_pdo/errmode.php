<?hh


<<__EntryPoint>>
function main_errmode() {
$db = getenv('HPHP_TEST_TMPDIR') . 'errmode.php.sq3';

$pdo = new PDO("sqlite:$db");
$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING);
$pdo->setAttribute(PDO::ATTR_ERRMODE, 77);
$pdo->exec('this is not a query');
unset($pdo);
unlink($db);
}
