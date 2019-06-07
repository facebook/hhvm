<?hh <<__EntryPoint>> function main() {
$ts = new MongoTimestamp();
var_dump(time() - $ts->sec <= 1);
}
