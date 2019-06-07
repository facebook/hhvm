<?hh <<__EntryPoint>> function main() {
$date = new MongoDate();
var_dump(time() - $date->sec <= 1);
var_dump($date->usec);
}
