<?hh
<<__EntryPoint>>
function main_entry(): void {
  // ±5 and ±10 (and any non-zero multiple of 5) is broken, but everything else
  // should already work correctly.
  $weekdays = range(-11, 11);
  $dates = vec['2012-03-29', '2012-03-30', '2012-03-31', '2012-04-01', '2012-04-02', '2012-04-03', '2012-04-04', '2012-04-05'];

  $header = vec[];

  foreach ($dates as $startdate) {
  	$date = new DateTime($startdate);

  	$header[] = $date->format('Y-m-d D');
  }

  echo '###  ', implode('  ', $header), "\n\n";

  foreach ($weekdays as $days) {
  	$line = vec[];

  	printf('%+3d  ', $days);

  	foreach ($dates as $startdate) {
  		$date = new DateTime($startdate);
  		$date->modify("{$days} weekdays");

  		$line[] = $date->format('Y-m-d D');
  	}

  	echo implode('  ', $line), "\n";
  }
}
