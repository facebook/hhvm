<?php
date_default_timezone_set('UTC');
$db = new DateTime( '2008-01-01' );
$de = new DateTime( '2008-12-31' );
$di = DateInterval::createFromDateString( 'first day of next month' );

foreach ( new DatePeriod( $db, $di, $de ) as $dt )
{
    echo $dt->modify( "3 tuesday" )->format( "l Y-m-d\n" );
}
?>

<?php
$db = new DateTime( '2007-12-31' );
$de = new DateTime( '2009-12-31 23:59:59' );
$di = DateInterval::createFromDateString( 'last thursday of next month' );

foreach ( new DatePeriod( $db, $di, $de, DatePeriod::EXCLUDE_START_DATE ) as $dt )
{
    echo $dt->format( "l Y-m-d H:i:s\n" );
}
?>