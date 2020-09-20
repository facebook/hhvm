<?hh

class MyDB extends MongoDB {
    public function __construct() {}
}
<<__EntryPoint>> function main(): void {
$db = new MyDB;

try {
    MongoDBRef::get($db, darray['$ref' => "", '$id' => 1]);
} catch (MongoException $e) {
    var_dump($e->getCode());
    var_dump($e->getMessage());
}
}
