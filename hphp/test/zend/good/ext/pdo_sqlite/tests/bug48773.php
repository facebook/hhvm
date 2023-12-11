<?hh

class bar extends PDOStatement {
    private function __construct() {
    }
}

class foo extends PDO {
    public $statementClass = 'bar';
    function __construct($dsn, $username, $password, $driver_options = dict[]) {
        $driver_options[PDO::ATTR_ERRMODE] = PDO::ERRMODE_EXCEPTION;
        parent::__construct($dsn, $username, $password, $driver_options);

        $this->setAttribute(PDO::ATTR_STATEMENT_CLASS, vec[$this->statementClass, vec[$this]]);
    }
}
<<__EntryPoint>> function main(): void {
$db = new foo('sqlite::memory:', '', '');
$stmt = $db->query('SELECT 1');
var_dump($stmt);
}
