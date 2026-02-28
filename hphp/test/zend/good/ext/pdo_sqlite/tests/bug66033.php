<?hh
class DBStatement extends PDOStatement {
    public $dbh;
    protected function __construct($dbh) {
        $this->dbh = $dbh;
        throw new Exception("Blah");
    }
}
<<__EntryPoint>> function main(): void {
$pdo = new PDO('sqlite::memory:', '', '');
$pdo->setAttribute(PDO::ATTR_STATEMENT_CLASS, vec['DBStatement',
    vec[$pdo]]);
$pdo->exec("CREATE TABLE IF NOT EXISTS messages (
    id INTEGER PRIMARY KEY,
    title TEXT,
    message TEXT,
    time INTEGER)");

try {
    $pdoStatement = $pdo->query("select * from messages");
} catch (Exception $e) {
    var_dump($e->getMessage());
}
}
