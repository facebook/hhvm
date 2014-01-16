<?php
$d = date_create( "Mon, 23 May 1955 00:00:00 +0200" );
var_dump( $d );
echo $d->format( DATE_ISO8601 ), "\n";
echo $d->format( 'U' ), "\n\n";

$d->setTimeZone( new DateTimeZone( 'Europe/Budapest' ) );
var_dump( $d );
echo $d->format( DATE_ISO8601 ), "\n\n";
echo $d->format( 'U' ), "\n\n";