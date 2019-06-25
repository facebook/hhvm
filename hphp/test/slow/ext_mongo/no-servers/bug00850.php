<?hh <<__EntryPoint>> function main(): void {
$mc = new MongoClient("", array("connect" => false));
var_dump($mc->__toString());
}
