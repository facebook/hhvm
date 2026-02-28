<?hh

<<__EntryPoint>>
function main_github_issue_2530_dateperiod_iteration() :mixed{
$begin = new DateTime( '2012-08-30' );
$end = new DateTime( '2012-08-31' );

$interval = new DateInterval('P1D');
$daterange = new DatePeriod($begin, $interval ,$end);

foreach($daterange as $date){
    echo $date->format("Ymd") . PHP_EOL;
}
}
