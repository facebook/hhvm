<?hh <<__EntryPoint>> function main(): void {
$date = new MongoDate(null, null);
printf("%d.%06d\n", $date->sec, $date->usec);

$date = new MongoDate('12345', '67890');
printf("%d.%06d\n", $date->sec, $date->usec);
}
