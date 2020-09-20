<?hh <<__EntryPoint>> function main(): void {
$mc = new MongoClient("", darray["connect" => false]);
var_dump($mc->__toString());
}
