<?hh

class bar extends PDOStatement {
    private function __construct() {
    }
}

class foo extends PDO {
    public $statementClass = 'bar';
    function __construct($dsn, $username, $password, $driver_options = varray[]) {
        $driver_options[PDO::ATTR_ERRMODE] = PDO::ERRMODE_EXCEPTION;
        parent::__construct($dsn, $username, $password, $driver_options);

        $this->setAttribute(PDO::ATTR_STATEMENT_CLASS, varray[$this->statementClass, varray[$this]]);
    }
}
<<__EntryPoint>> function main(): void {
$db = new foo('sqlite::memory:', '', '');
$stmt = $db->query('SELECT 1');
var_dump($stmt);
}
