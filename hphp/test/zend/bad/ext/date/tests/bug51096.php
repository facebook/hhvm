<?php
$tests = array(
	'first day',
	'last day',
	'next month',
	'first day next month',
	'last day next month',
	'first day of next month',
	'last day of next month'
);

foreach ( $tests as $test )
{
	$result = date_parse( $test );
	$rel = $result['relative'];
	echo $test, "\n- month: ", $rel['month'], '; day: ', $rel['day'],
		 '; first-day-of: ', isset( $rel['first_day_of_month'] ) ? 'true' : 'false', 
		 '; last-day-of: ', isset( $rel['last_day_of_month'] ) ? 'true' : 'false', "\n";
	$date = new DateTime( '2010-03-06 15:21 UTC' );
	echo '- ', $date->format( DateTime::ISO8601 );
	$date->modify( $test );
	echo ' -> ', $date->format( DateTime::ISO8601 ), "\n\n";
}
?>