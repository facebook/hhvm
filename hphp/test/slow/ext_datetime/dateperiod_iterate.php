<?hh


<<__EntryPoint>>
function main_dateperiod_iterate() :mixed{
$begin = new DateTime( '2012-08-01' );
$end = new DateTime( '2012-08-31' );
$end = $end->modify( '+1 day' );

$interval = new DateInterval('P1D');
$daterange = new DatePeriod($begin, $interval ,$end);

$v = vec[];
foreach($daterange as $date){
  echo $date->format("Ymd") . "\n";
  $v[] = $date;
}
foreach ($v as $date) {
  echo $date->format("Ymd") . "\n";
}
}
