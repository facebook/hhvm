<?php
date_default_timezone_set('UTC');
$db1 = new DateTimeImmutable( '2008-01-01' );
$db2 = new DateTime( '2008-01-01' );
$de = new DateTime( '2008-03-31' );
$di = DateInterval::createFromDateString( 'first day of next month' );

foreach ( new DatePeriod( $db1, $di, $de ) as $dt )
{
	echo get_class( $dt ), "\n";
	echo $dt->format( "l Y-m-d\n" );
    echo $dt->modify( "3 tuesday" )->format( "l Y-m-d\n" );
	echo $dt->format( "l Y-m-d\n\n" );
}

foreach ( new DatePeriod( $db2, $di, $de ) as $dt )
{
	echo get_class( $dt ), "\n";
	echo $dt->format( "l Y-m-d\n" );
    echo $dt->modify( "3 tuesday" )->format( "l Y-m-d\n" );
	echo $dt->format( "l Y-m-d\n\n" );
}
?>