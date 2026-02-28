<?hh
class DbGow {

    public function query() :mixed{
        throw new Exception;
    }

    public function select() :mixed{
        return new DbGowRecordSet($this->query());
    }

    public function select2() :mixed{
        new DbGowRecordSet($this->query());
    }

}

class DbGowRecordSet {

    public function __construct($resource) {
    }

}
<<__EntryPoint>> function main(): void {
$db = new DbGow;

try {
    $rs = $db->select();
} catch(Exception $e) {
    echo "ok\n";
}

try {
    $db->select2();
} catch(Exception $e) {
    echo "ok\n";
}
}
