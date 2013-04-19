<?php
date_default_timezone_set('Europe/Oslo');

try
{
	$dp = new DatePeriod('2D');
}
catch ( Exception $e )
{
	echo $e->getMessage(), "\n";
}

$begin = new DateTime( "2008-07-20T22:44:53+0200" );
$interval = DateInterval::createFromDateString( "1 day" );

$dp = new DatePeriod( $begin, $interval, 10 );
foreach ( $dp as $d )
{
	var_dump ($d->format( DATE_ISO8601 ) ); 
}

?>