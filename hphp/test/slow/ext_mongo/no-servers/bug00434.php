<?hh
class m extends Mongo { function __construct() {} }
<<__EntryPoint>> function main(): void {
try {
    $m = new m;
    $m->connect();
} catch(Exception $e) {
    var_dump($e->getMessage());
}
echo "===DONE===\n";
}
