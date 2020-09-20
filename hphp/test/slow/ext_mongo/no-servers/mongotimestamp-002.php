<?hh <<__EntryPoint>> function main(): void {
$ts = new MongoTimestamp();
var_dump(time() - $ts->sec <= 1);
}
