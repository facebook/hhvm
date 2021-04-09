<?hh
<<__EntryPoint>> function main(): void {
$date1 = "2017-01-08"; // this is a Sunday
$date = new DateTime($date1);
$date->modify('Friday this week');
$dateFormat = $date->format('Y-m-d');
echo $dateFormat, "\n";
}
